#include "CUdpEventSegment.h"

#include <arpa/inet.h>
#include <cerrno>
#include <cstring>
#include <netinet/in.h>
#include <stdexcept>
#include <sys/socket.h>
#include <unistd.h>

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
    throw std::runtime_error("CUdpEventSegment::onBegin - socket() failed: " +
                             std::string(strerror(errno)));
  }

  int reuse = 1;
  setsockopt(m_socket, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse));

  sockaddr_in addr;
  memset(&addr, 0, sizeof(addr));
  addr.sin_family = AF_INET;
  addr.sin_port = htons(m_port);
  addr.sin_addr.s_addr =
      m_bindAddr.empty() ? INADDR_ANY : inet_addr(m_bindAddr.c_str());

  if (bind(m_socket, reinterpret_cast<sockaddr *>(&addr), sizeof(addr)) < 0) {
    int err = errno;
    close(m_socket);
    m_socket = -1;
    throw std::runtime_error("CUdpEventSegment::onBegin - bind() failed: " +
                             std::string(strerror(err)));
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