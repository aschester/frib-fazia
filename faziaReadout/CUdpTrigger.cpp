/**
 * @file CUdpTrigger.cpp
 * @brief This file contains the implementation of the CUdpTrigger class, which
 * implements a UDP-based event trigger for the SBS readout framework.
 */

#include "CUdpTrigger.h"
#include "CUdpEventSegment.h"

#include <poll.h>

/**
 * @details
 * The CUdpTrigger class implements a UDP-based event trigger for the SBS
 * readout framework. It checks if there is data available on the UDP socket
 * associated with the event segment. The `operator()` method uses the `poll()`
 * system call to poll the socket for incoming data without blocking. If data is
 * available, it returns true, indicating that the trigger condition is met;
 * otherwise, it returns false. The trigger will ignore errors on `poll()`,
 * return false, an keep trying.
 */
bool CUdpTrigger::operator()() {
  int fd = m_segment.getSocketFd();
  if (fd < 0) {
    return false; // Socket not open (e.g. between runs)
  }
  pollfd pfd{fd, POLLIN, 0};
  int rc = poll(&pfd, 1, 0);

  // Only return true if there is data available to read on the socket and the
  // socket is still open:
  return (rc > 0) && (pfd.revents & POLLIN);
}