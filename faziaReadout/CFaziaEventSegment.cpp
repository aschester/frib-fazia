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

/**
 * @details
 * Invokes CUdpEventSegment class constructor.
 */
CFaziaEventSegment::CFaziaEventSegment(unsigned short port,
                                       const std::string &bindAddr,
                                       uint32_t sourceId)
    : CUdpEventSegment(port, bindAddr, sourceId) {}

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
  uint64_t timestamp;
  uint64_t evtSize;
  static const size_t HEADER_SIZE = sizeof(timestamp) + sizeof(evtSize);

  uint8_t raw[MAX_FAZIA_EVENT_BYTES];
  ssize_t n = receivePacket(raw, sizeof(raw));

  if (n <= 0) {
    reject();
    return 0;
  }

  if (static_cast<size_t>(n) < HEADER_SIZE) {
    std::cerr << "CFaziaEventSegment: packet too short for header (" << n
              << " bytes) - dropped" << std::endl;
    reject();
    return 0;
  }

  memcpy(&timestamp, raw, sizeof(timestamp));
  memcpy(&evtSize, raw + sizeof(timestamp), sizeof(evtSize));

  size_t received = static_cast<size_t>(n) - HEADER_SIZE;
  if (evtSize != received) {
    std::cerr << "CFaziaEventSegment: evtSize field (" << evtSize
              << ") doesn't match actual payload received (" << received
              << ") - dropped" << std::endl;
    reject();
    return 0;
  }

  // Whole packet size, header + payload rounded up to 16-bit words if the byte
  // count is odd. Odd byte is padded with zero. This may not be needed but
  // (I think) its harmless:
  size_t nbytes = static_cast<size_t>(n);
  size_t words = (nbytes + 1) / sizeof(uint16_t);
  if (words > maxwords) {
    throw CRangeError(0, maxwords, words,
                      "CFaziaEventSegment::read - event too large for buffer");
  }

  memcpy(pBuffer, raw, nbytes);
  // Zero-pad the odd byte to make it even for 16-bit word alignment:
  if (nbytes & 1) {
    static_cast<uint8_t *>(pBuffer)[nbytes] = 0;
  }

  setSourceId(getSourceId());
  setTimestamp(timestamp);

  return words;
}