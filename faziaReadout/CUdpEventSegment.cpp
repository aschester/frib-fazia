#include "CUdpEventSegment.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <iostream>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

#include <ErrnoException.h>

CUdpEventSegment::CUdpEventSegment(unsigned short port,
                                   const std::string &bindAddr,
                                   uint32_t sourceId)
    : m_bindAddr(bindAddr), m_port(port), m_socket(-1), m_sourceId(sourceId) {}

CUdpEventSegment::~CUdpEventSegment() { closeSocket(); }

void CUdpEventSegment::onBegin() { openSocket(); }
void CUdpEventSegment::onEnd() { closeSocket(); }

void CUdpEventSegment::openSocket() {
  if (m_socket != -1) {
    return;
  }

  m_socket = socket(AF_INET, SOCK_DGRAM, 0);
  if (m_socket < 0) {
    // socket() can fail for runtime reasons (fd/resource exhaustion) unrelated
    // to configuration. The begin-run handler reports exceptions generically,
    // so log the cause before throwing.
    std::cerr << "CUdpEventSegment::openSocket - socket() failed: "
              << strerror(errno) << std::endl;
    throw CErrnoException("CUdpEventSegment::openSocket - socket() failed");
  }

  int reuse = 1;
  setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_port);

  // FAZIA_IP is validated upstream in faziaReadout::SetupReadout, so the
  // address string is assumed well-formed here; empty means bind all
  // interfaces.
  if (m_bindAddr.empty()) {
    addr.sin_addr.s_addr = INADDR_ANY;
  } else {
    inet_pton(AF_INET, m_bindAddr.c_str(), &addr.sin_addr);
  }

  if (bind(m_socket, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    // bind() can still fail at runtime with valid config: EADDRINUSE (port
    // taken), EADDRNOTAVAIL (address not on a local interface), EACCES
    // (privileged port). Preserve errno across close() for CErrnoException,
    // and log the cause since the begin-run handler is generic.
    int err = errno;
    close(m_socket);
    m_socket = -1;
    errno = err;
    std::cerr << "CUdpEventSegment::openSocket - bind() failed: "
              << strerror(err) << std::endl;
    throw CErrnoException("CUdpEventSegment::openSocket - bind() failed");
  }
}

void CUdpEventSegment::closeSocket() {
  if (m_socket != -1) {
    close(m_socket);
    m_socket = -1;
  }
}

ssize_t CUdpEventSegment::receivePacket(void *pBuffer, size_t buflen) {
  if (m_socket == -1) {
    return 0;
  }

  ssize_t n =
      recvfrom(m_socket, pBuffer, buflen, MSG_DONTWAIT, nullptr, nullptr);

  if (n < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0;
    }
    return -1;
  }

  return n;
}