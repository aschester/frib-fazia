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
    : m_lastCounter(0), m_haveCounter(false),
      CUdpEventSegment(port, bindAddr, sourceId) {}

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

  if (nbytes < sizeof(uint32_t)) {
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

ParseResult CFaziaEventSegment::parseEvent(const uint8_t *data,
                                           std::size_t nbytes) {
  ParseResult result;
  size_t i = 0;
  size_t nwords = nbytes / sizeof(uint16_t);

  while (i < nwords) {
    const uint16_t w = data[i];

    if ((w & format::EOE_MASK) == format::EOE_VALUE) {
      result.sawEnd = true;
      break;
    }
    if (w == format::PADDING) {
      ++i;
      continue;
    }
    if ((w & format::HDR5_MASK) == format::TELHDR_VALUE) {
      i += 1 + format::TRIGGER_WORDS;
      continue;
    }

    if ((w & format::NIBBLE_MASK) == format::TAG_VALUE) {
      if (i + 1 >= nwords) {
        break;
      }
      const std::size_t taglen = data[i + 1];
      const std::size_t payload = i + 2;
      if (payload + taglen > nwords) {
        break;
      }
      if (w == format::TAG_FRIB_TS && !result.haveTimestamp) {
        uint64_t value = 0;
        for (std::size_t k = 0; k < taglen; ++k)
          value = (value << format::TS_BITS_PER_WORD) | data[payload + k];
        result.timestamp = value;
        result.haveTimestamp = true;
      }
      i = payload + taglen;
      continue;
    }
    ++i;
  }

  return result;
}

} // namespace fazia