/**
 * @file CUdpEventSegment.h
 * @brief This file contains the definition of the CUdpEventSegment class, which
 * implements a UDP-based event segment for the SBS readout framework.
 */

#ifndef CUDPEVENTSEGMENT_H
#define CUDPEVENTSEGMENT_H

#include <CEventSegment.h>
#include <cstdint>
#include <string>

/**
 * @class CUdpEventSegment
 * @brief This class implements a UDP-based event segment for the SBS readout
 * framework. It provides functionality to open and close a UDP socket, receive
 * packets from the socket, and manage the source ID of the events. The class is
 * designed to be inherited by specific event segment implementations that
 * handle the actual event data. This is a pure virtual class, and the read()
 * method must be implemented by derived classes to define how event data is
 * read from the UDP socket.
 */

class CUdpEventSegment : public CEventSegment {
private:
  std::string m_bindAddr; //<! The IP address to bind to for receiving event
                          //<! data. If empty, bind to all interfaces.
  unsigned short m_port;  //!< The UDP port to bind to for receiving event data.
  int m_socket; //<! The file descriptor for the UDP socket used to receive
                //<! event data.
  uint32_t m_sourceId; //<! The source ID for the events.

public:
  /**
   * @brief Constructor for the CUdpEventSegment class.
   * @param port The UDP port to bind to for receiving event data
   * (default=50000).
   * @param bindAddr The IP address to bind to for receiving event data. If
   * empty, bind to all interfaces (default="").
   * @param sourceId The source ID for the event segment, used to identify the
   * source of the events in the data stream (default=0).
   */
  CUdpEventSegment(unsigned short port, const std::string &bindAddr = "",
                   uint32_t sourceId = 0);
  /** @brief Destructor for the CUdpEventSegment class. */
  virtual ~CUdpEventSegment();

  /**
   * @brief Called at the beginning of a run to open the UDP socket for
   * receiving event data.
   */
  virtual void onBegin();
  /**
   * @brief Called at the end of a run to close the UDP socket for receiving
   * event data.
   */
  virtual void onEnd();
  /**
   * @brief Pure virtual function to read event data from the UDP socket.
   * Implementation is left to derived classes.
   * @param pBuffer The buffer to store the received event data.
   * @param maxwords The maximum number of words to read.
   * @return The number of words read.
   */
  virtual size_t read(void *pBuffer, size_t maxwords) = 0;

  /**
   * @brief Get the file descriptor of the UDP socket.
   * @return The file descriptor of the UDP socket, or -1 if the socket is not
   * open.
   */
  int getSocketFd() const { return m_socket; }
  /**
   * @brief Get the source ID for the event segment.
   * @return The source ID for the event segment.
   */
  uint32_t getSourceId() const { return m_sourceId; }

protected:
  /**
   * @brief Receive a packet from the UDP socket.
   * @param pBuffer The buffer to store the received packet data.
   * @param maxbytes The maximum number of bytes to read.
   * @return The number of bytes read, or -1 on error.
   */
  ssize_t receivePacket(void *pBuffer, size_t maxbytes);

private:
  /** @brief Open the UDP socket for receiving event data. */
  void openSocket();
  /** @brief Close the UDP socket for receiving event data. */
  void closeSocket();
};

#endif