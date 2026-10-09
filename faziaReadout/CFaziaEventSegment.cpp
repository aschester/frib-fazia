/**
 * @file CFaziaEventSegment.cpp
 * @brief This file contains the implementation of the CFaziaEventSegment class,
 * which implements the event segment for the FAZIA detector based on the SBS
 * readout framework.
 */

#include "CFaziaEventSegment.h"

#include <cstring>
#include <iostream>

#include <RangeError.h>

#include "FaziaFormat.h"

namespace fazia {

/**
 * @details
 * Invokes CUdpEventSegment class constructor.
 */
CFaziaEventSegment::CFaziaEventSegment(unsigned short port,
                                       const std::string &bindAddr,
                                       uint32_t sourceId)
    : CUdpEventSegment(port, bindAddr, sourceId), m_lastCounter(0),
      m_haveCounter(false) {}

void CFaziaEventSegment::onBegin() {
  m_lastCounter = 0;
  m_haveCounter = false;
  CUdpEventSegment::onBegin();
}

/**
 * @details
 * Reads event data from the UDP socket into the provided buffer. It first
 * receives a packet from the socket, checks if the packet is large enough to
 * contain the header, and then extracts the timestamp and event size from
 * the header. It verifies that the event size matches the actual payload
 * received. If everything is valid, it copies the entire packet (header +
 * payload) into the provided buffer, sets the source ID and timestamp for the
 * event, and returns the number of 16-bit words read into the buffer. If any
 * checks fail, it rejects the event and returns 0.
 * @todo (ASC 9/9/26): Need to figure out how to handle the case where the event
 * size is larger than the buffer size and is wrapped across multiple UDP
 * packets.
 */
size_t CFaziaEventSegment::read(void *pBuffer, size_t maxwords) {
  uint8_t raw[MAX_EVENT_BYTES];
  uint32_t counter = 0;

  ssize_t n = receivePacket(raw, sizeof(raw));
  if (n <= 0) {
    reject();
    return 0;
  }
  size_t nbytes = static_cast<size_t>(n);

  if (nbytes < sizeof(counter)) {
    std::cerr << "fazia::CFaziaEventSegment: datagram too short for counter ("
              << nbytes << " bytes) - dropped" << std::endl;
    reject();
    return 0;
  }

  // Check for packet continuity and warn if packets are dropped. The counter is
  // the first 4 bytes of the datagram:

  std::memcpy(&counter, raw, sizeof(counter));
  checkPacketCounter(counter);

  // Parse the FAZIA words (after the 4-byte counter) for FRIB timestamp + EOE:

  ParseResult p = parseEvent(raw + sizeof(counter), nbytes - sizeof(counter));

  if (!p.haveTimestamp) {
    std::cerr << "fazia::CFaziaEventSegment: no FRIB timestamp in datagram "
                 "(counter "
              << counter << ", " << nbytes << " bytes) - dropped" << std::endl;
    reject();
    return 0; // Device keeps running; just skip this event
  }

  if (!p.sawEnd) {
    // No EOE before end of datagram: event likely spans datagrams (not yet
    // handled) or data is corrupt. We have a valid timestamp, so emit with a
    // warning. Change to reject()/return 0 to drop these instead.
    // TODO(reassembly): accumulate across datagrams, split on EOE, and bound
    // the accumulator so a never-arriving EOE can't grow without limit.
    std::cerr << "fazia::CFaziaEventSegment: no EOE in datagram (counter "
              << counter << ") - possible multi-datagram event; emitting as-is"
              << std::endl;
  }

  // Whole datagram (counter + padding + words) becomes the ring-item body so
  // downstream can strip it and use the counter for diagnostics:

  size_t words = (nbytes + 1) / sizeof(uint16_t);
  if (words > maxwords) {
    throw CRangeError(
        0, maxwords, words,
        "fazia::CFaziaEventSegment::read - event too large for buffer");
  }
  std::memcpy(pBuffer, raw, nbytes);
  if (nbytes & 1) {
    static_cast<uint8_t *>(pBuffer)[nbytes] = 0; // Zero the odd byte
  }

  setSourceId(getSourceId());
  setTimestamp(p.timestamp);

  return words;
}

///
// Private functions
//

void CFaziaEventSegment::checkPacketCounter(uint32_t counter) {
  if (m_haveCounter && counter != m_lastCounter + 1) {
    uint32_t missing =
        counter - m_lastCounter - 1; // uint32 wraps intentionally
    std::cerr << "fazia::CFaziaEventSegment: packet counter gap - expected "
              << (m_lastCounter + 1) << ", got " << counter << " (" << missing
              << " packet(s) missing or reordered)" << std::endl;
  }
  m_lastCounter = counter;
  m_haveCounter = true;
}

/**
 * @details
 * Walk one FAZIA event (packet counter already stripped) and report the FRIB
 * timestamp (if present) and whether an EOE closed the event. Some special
 * cases to consider:
 * - TELHDR..DETHDR gap: The TELHDR header is followed by 3 words of trigger
 *   information before the DETHDR header. This gap is skipped over when
 *   parsing. The TELHDR trigger words are explicitly skipped, DETHDR just falls
 *   though as a normal header.
 * - Padding: The PADDING word (0x8080) is ignored and skipped over when
 *   parsing.
 * - Tags: The TAG_FRIB_TS tag (0x7300) is used to extract the FRIB timestamp.
 *   All other tags are ignored. The tag length is determined by the word
 *   following the tag header, and the payload is extracted accordingly
 */
ParseResult CFaziaEventSegment::parseEvent(const void *data,
                                           std::size_t nbytes) {

  const uint16_t *data16 = static_cast<const uint16_t *>(data);
  size_t nwords = nbytes / sizeof(uint16_t);

  ParseResult result;
  size_t i = 0;
  while (i < nwords) {
    const uint16_t w = data16[i];

    if ((w & format::EOE_MASK) == format::EOE_VALUE) {
      result.sawEnd = true;
      break;
    }
    if (w == format::PADDING) {
      i++;
      continue;
    }
    if ((w & format::HDR5_MASK) == format::TELHDR_VALUE) {
      i += 1 + format::TRIGGER_WORDS;
      continue;
    }

    if ((w & format::NIBBLE_MASK) == format::TAG_VALUE) {

      // We have a tag. If its an FRIB timestamp tag, extract the payload as a
      // 15-bit-per-word value as the docs say:

      if (i + 1 >= nwords) {
        break;
      }
      const size_t taglen = data16[i + 1]; // In 16-bit words
      const size_t payload = i + 2; // Index of the start of the tag's payload
      if (payload + taglen > nwords) {
        break;
      }
      if (w == format::TAG_FRIB_TS && !result.haveTimestamp) {
        uint64_t value = 0;
        for (size_t j = 0; j < taglen; j++) {
          value = (value << format::TS_BITS_PER_WORD) | data16[payload + j];
        }
        result.timestamp = value;
        result.haveTimestamp = true;
      }
      //
      // More tags could be handled here if needed, but for now we just skip
      // over them.
      //
      i = payload + taglen; // Index in the data after the tag's payload
      continue;
    }
    i++; // Fallthrough: skip this word
  }

  return result;
}

} // namespace fazia