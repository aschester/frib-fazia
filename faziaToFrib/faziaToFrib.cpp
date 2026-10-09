/**
 * @file faziaToFrib.cpp
 * @brief Read FAZIA framed events on stdin, write NSCLDAQ ring items to a
 * configurable sink (ring buffer, file, or stdout).
 */

#include <cstring>
#include <iostream>
#include <memory>
#include <string>

#include <CDataSink.h>
#include <CDataSinkFactory.h>
#include <CPhysicsEventItem.h>
#include <DataFormat.h>
#include <Exception.h>
#include <io.h>

#include "FaziaFormat.h"
#include "faziatofribargs.h"

using namespace fazia;

/**
 * @brief Program main
 * @details
 * FAZIA writes one record per event on stdout, which we read on stdin:
 *   [ 8 bytes       : uint64_t timestamp ]
 *   [ 8 bytes       : uint64_t evtSize   ]   (payload size, in bytes)
 *   [ evtSize bytes : payload            ]
 * Each record is transformed into a PHYSICS_EVENT ring item and handed to the
 * sink selected by --sink. The sink is any NSCLDAQ sink URI understood by
 * CDataSinkFactory: '-' for stdout, 'file:///path' for a file, or
 * 'tcp://localhost/ringname' for a ring buffer (ring buffers _must_ be
 * localhost).
 * @note (ASC 10/8/26): The wire format of the FAZIA record is assumed based on
 * tests with the FAZIA DAQ run at FRIB in September, 2026. Subject to change in
 * the future, not documented, etc.
 * @return EXIT_SUCCESS on success, otherwise EXIT_FAILURE
 */
int main(int argc, char *argv[]) {
  gengetopt_args_info parser;
  if (cmdline_parser(argc, argv, &parser) != 0) {
    return EXIT_FAILURE;
  }

  try {
    CDataSinkFactory factory;
    std::unique_ptr<CDataSink> pSink(
        factory.makeSink(std::string(parser.sink_arg)));

    while (true) {
      uint64_t timestamp;
      uint64_t evtSize;
      size_t n;

      n = io::readData(STDIN_FILENO, &timestamp, sizeof(timestamp));
      if (n == 0) {
        break; // Upstream closed stdin
      }
      if (n != sizeof(timestamp)) {
        std::cerr << "ERROR: truncated timestamp field on stdin" << std::endl;
        cmdline_parser_free(&parser);
        return EXIT_FAILURE;
      }

      n = io::readData(STDIN_FILENO, &evtSize, sizeof(evtSize));
      if (n != sizeof(evtSize)) {
        std::cerr << "ERROR: truncated size field on stdin" << std::endl;
        cmdline_parser_free(&parser);
        return EXIT_FAILURE;
      }

      // Source Id = 6 ("F" for Fazia) is hardcoded for now, barrier = 0 for
      // PHYSICS_EVENT data:

      auto pItem = std::unique_ptr<CPhysicsEventItem>(
          new CPhysicsEventItem(timestamp, 6, 0, MAX_EVENT_BYTES + 128));

      // Copy the timestamp and payload size into the ring item body:

      auto pBody = reinterpret_cast<uint64_t *>(pItem->getBodyPointer());
      *pBody++ = timestamp;
      *pBody++ = evtSize;
      auto pPayload = reinterpret_cast<uint8_t *>(pBody);

      // Read FAZIA payload:

      n = io::readData(STDIN_FILENO, pPayload, evtSize);
      if (n != evtSize) {
        std::cerr << "ERROR: truncated payload on stdin (expected " << evtSize
                  << " bytes, got " << n << ")" << std::endl;
        cmdline_parser_free(&parser);
        return EXIT_FAILURE;
      }
      pPayload += evtSize;

      pItem->setBodyCursor(pPayload);
      pItem->updateSize();

      pSink->putItem(*pItem);
    }
  } catch (int &e) {
    std::cerr << "I/O error: " << strerror(e) << std::endl;
    cmdline_parser_free(&parser);
    return EXIT_FAILURE;
  } catch (CException &e) {
    std::cerr << "ERROR: NSCLDAQ exception: " << e.ReasonText() << std::endl;
    cmdline_parser_free(&parser);
    return EXIT_FAILURE;
  } catch (std::exception &e) {
    std::cerr << "ERROR: C++ exception: " << e.what() << std::endl;
    cmdline_parser_free(&parser);
    return EXIT_FAILURE;
  } catch (...) {
    std::cerr << "ERROR: unexpected exception" << std::endl;
    cmdline_parser_free(&parser);
    return EXIT_FAILURE;
  }

  cmdline_parser_free(&parser);
  return EXIT_SUCCESS;
}
