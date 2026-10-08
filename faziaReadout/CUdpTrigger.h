/**
 * @file CUdpTrigger.h
 * @brief This file contains the definition of the CUdpTrigger class, which
 * implements a UDP-based event trigger for the SBS readout framework.
 */

#ifndef CUDPTRIGGER_H
#define CUDPTRIGGER_H

#include <CEventTrigger.h>

class CUdpEventSegment;

/**
 * @class CUdpTrigger
 * @brief This class implements a UDP-based event trigger for the SBS readout.
 */

class CUdpTrigger : public CEventTrigger {
private:
  CUdpEventSegment &m_segment; //!< Our event segment.

public:
  /**
   * @brief Constructor for the CUdpTrigger class.
   * @param segment The event segment to trigger on.
   */
  explicit CUdpTrigger(CUdpEventSegment &segment) : m_segment(segment) {};
  /**
   * @brief Operator() to check if the trigger condition is met.
   * @return True if the trigger condition is met, false otherwise.
   */
  virtual bool operator()();
};

#endif