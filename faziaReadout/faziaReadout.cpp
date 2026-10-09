/**
 * @file faziaReadout.cpp
 * @brief This file contains the implementation of the faziaReadout class.
 */

#include "faziaReadout.h"

#include <arpa/inet.h>

#include <CExperiment.h>
#include <CInvalidArgumentException.h>
#include <CTimedTrigger.h>
#include <RangeError.h>
#include <TCLInterpreter.h>
#include <config.h>

#include "CFaziaEventSegment.h"
#include "CUdpTrigger.h"
#include "FaziaFormat.h"

namespace fazia {

/**
 * @details
 * This function sets up the readout for the FAZIA detector. It creates a
 * CFaziaEventSegment and a CUdpTrigger, and adds them to the experiment. The
 * UDP port and bind address can be configured via the FAZIA_PORT and FAZIA_IP
 * environment variables, respectively. If these variables are not set, default
 * values of 50000 for the port and an empty string for the bind address (which
 * binds to all interfaces) are used. The source ID for the event segment is
 * obtained from the experiment object an is set on the command line when
 * starting the readout program.
 */
void faziaReadout::SetupReadout(CExperiment *pExperiment) {
  CReadoutMain::SetupReadout(pExperiment);

  // Experiment buffer size must be at least as large as the largest FAZIA event
  // we accept:

  pExperiment->setBufferSize(MAX_EVENT_BYTES + 128);

  unsigned short port = 50000; // FAZIA's default UDP port
  std::string bindAddr;        // empty = bind to all interfaces

  if (const char *pPort = getenv("FAZIA_PORT")) {
    char *end = nullptr;
    unsigned long myPort = strtoul(pPort, &end, 0);
    if (*pPort == '\0' || *end != '\0' || myPort == 0 || myPort > 65535) {
      throw CRangeError(1, 65535, static_cast<long>(myPort),
                        std::string(" while parsing FAZIA_PORT (got '") +
                            pPort + "')");
    }
    port = static_cast<unsigned short>(myPort);
  }

  if (const char *pIp = getenv("FAZIA_IP")) {
    bindAddr = pIp;
    in_addr tmp;
    if (inet_pton(AF_INET, bindAddr.c_str(), &tmp) != 1) {
      throw CInvalidArgumentException(
          bindAddr, " is not a valid IPv4 dotted-quad address",
          "faziaReadout::SetupReadout - parsing FAZIA_IP");
    }
  }

  uint32_t sourceId = pExperiment->getSourceId();

  static CFaziaEventSegment faziaSegment(port, bindAddr, sourceId);
  static CUdpTrigger faziaTrigger(faziaSegment);

  pExperiment->AddEventSegment(&faziaSegment);
  pExperiment->EstablishTrigger(&faziaTrigger);
}

void faziaReadout::SetupScalers(CExperiment *pExperiment) {
  CReadoutMain::SetupScalers(pExperiment); // Default 2s-periodic scaler trigger

  // Add scaler modules here if you have any.
}

void faziaReadout::addCommands(CTCLInterpreter *pInterp) {
  CReadoutMain::addCommands(pInterp);
}

void faziaReadout::SetupRunVariables(CTCLInterpreter *pInterp) {
  CReadoutMain::SetupRunVariables(pInterp);
}

void faziaReadout::SetupStateVariables(CTCLInterpreter *pInterp) {
  CReadoutMain::SetupStateVariables(pInterp);
}

} // namespace fazia

CTCLApplication *gpTCLApplication = new fazia::faziaReadout;