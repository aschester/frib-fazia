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
size_t CFaziaEventSegment::read(void *pBuffer, size_t maxwords) { return 1; }

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
      if (i + 1 >= nwords)
        break;
      const std::size_t taglen = data[i + 1];
      const std::size_t payload = i + 2;
      if (payload + taglen > nwords)
        break;
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