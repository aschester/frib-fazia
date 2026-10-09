/**
 * @file CFaziaEventSegment.h
 * @brief This file contains the definition of the CFaziaEventSegment class,
 * which implements the event segment for the FAZIA detector based on the SBS
 * readout framework.
 */

#ifndef CFAZIAEVENTSEGMENT_H
#define CFAZIAEVENTSEGMENT_H

#include "CUdpEventSegment.h"

#include <cstdint>

namespace fazia {

/** @brief Result of walking one FAZIA event. */
struct ParseResult {
  bool haveTimestamp = false; //<! A FRIB-timestamp tag was decoded
  uint64_t timestamp = 0;     //<! Valid only when haveTimestamp
  bool sawEnd = false;        //<! An EOE word terminated the event
};

/**
 * @class CFaziaEventSegment
 * @brief This class implements the event segment for the FAZIA detector based
 * on the SBS readout framework. It inherits from CUdpEventSegment and adds
 * functionality specific to the FAZIA detector, such as handling the source ID
 * and timestamp of events.
 */

class CFaziaEventSegment : public CUdpEventSegment {
private:
  uint32_t m_lastCounter;
  bool m_haveCounter;

public:
  /**
   * @brief Constructor for the CFaziaEventSegment class.
   * @param port The UDP port to bind to for receiving event data
   * (default=50000).
   * @param bindAddr The IP address to bind to for receiving event data. If
   * empty, bind to all interfaces (default="").
   * @param sourceId The source ID for the event segment, used to identify the
   * source of the events in the data stream (default=0).
   */
  explicit CFaziaEventSegment(unsigned short port = 50000,
                              const std::string &bindAddr = "",
                              uint32_t sourceId = 0);

  virtual void onBegin();
  /**
   * @brief Reads event data from the UDP socket into the provided buffer.
   * @param pBuffer Pointer to the buffer where the event data will be stored.
   * @param maxwords The maximum number of 16-bit words to read into the buffer.
   * @return The number of 16-bit words read into the buffer, or 0 if no data
   * was available, or a negative value if an error occurred.
   */
  virtual size_t read(void *pBuffer, size_t maxwords);

private:
  /**
   * @brief Check the packet counter and warn if packets are dropped.
   * @param counter The packet counter. Compared to the last received counter.
   */
  void checkPacketCounter(uint32_t counter);
  /// Walk one FAZIA event (packet counter already stripped) and report the FRIB
  /// timestamp (if present) and whether an EOE closed the event. Static and
  /// side-effect free, so it can be unit-tested on a captured event directly.
  /** @brief Parse a FAZIA event and extract its metadata.
   * @param data Pointer to the event data.
   * @param nbytes The number of bytes in the event data.
   * @return The result of the parsing operation.
   */
  ParseResult parseEvent(const uint8_t *data, std::size_t nbytes);
};

} // namespace fazia

#endif