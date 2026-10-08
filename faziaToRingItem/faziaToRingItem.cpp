/**
 * @file faziaToRingItem.cpp
 * @brief Read FAZIA framed events on stdin, write NSCLDAQ ring items to stdout.
 */

#include <cstring>
#include <iostream>
#include <memory>

#include <CPhysicsEventItem.h>
#include <DataFormat.h>
#include <io.h>

/**
 * @brief Program main
 * @details
 * FAZIA writes one record per event on stdin:
 *   [8 bytes: uint64_t timestamp]
 *   [8 bytes: uint64_t evtSize]   (payload size, in bytes)
 *   [evtSize bytes: payload]
 * @return EXIT_SUCCESS on success, otherwise EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
  try {
    while (true) {
      uint64_t timestamp;
      uint64_t evtSize;
      size_t n;

      n = io::readData(STDIN_FILENO, &timestamp, sizeof(timestamp));
      if (n == 0)
        continue; // Nothing written yet - wait for the next event.
      if (n != sizeof(timestamp)) {
        std::cerr << "ERROR: truncated timestamp field on stdin" << std::endl;
        return EXIT_FAILURE;
      }

      n = io::readData(STDIN_FILENO, &evtSize, sizeof(evtSize));
      if (n != sizeof(evtSize)) {
        std::cerr << "ERROR: truncated size field on stdin" << std::endl;
        return EXIT_FAILURE;
      }

      // Source Id = 6 ("F" for Fazia) is hardcoded for now, barrier = 0 for
      // PHYSICS_EVENT data:
      auto pItem = std::unique_ptr<CPhysicsEventItem>(
          new CPhysicsEventItem(timestamp, 6, 0));

      auto pBody = reinterpret_cast<uint8_t *>(pItem->getBodyPointer());
      n = io::readData(STDIN_FILENO, pBody, evtSize);
      if (n != evtSize) {
        std::cerr << "ERROR: truncated payload on stdin (expected " << evtSize
                  << " bytes, got " << n << ")" << std::endl;
        return EXIT_FAILURE;
      }
      pBody += evtSize;

      pItem->setBodyCursor(pBody);
      pItem->updateSize();

      io::writeData(STDOUT_FILENO, pItem->getItemPointer(),
                    pItem->getItemPointer()->s_header.s_size);
    }
  } catch (int &e) {
    std::cerr << "I/O error: " << strerror(e) << std::endl;
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "ERROR: unexpected exception" << std::endl;
    return EXIT_FAILURE;
  }

  return EXIT_SUCCESS;
}
