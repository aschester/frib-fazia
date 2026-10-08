/**
 * @file faziaReadout.cpp
 * @brief This file contains the implementation of the faziaReadout class.
 */

#include "faziaReadout.h"

#include <CExperiment.h>
#include <CTimedTrigger.h>
#include <TCLInterpreter.h>
#include <config.h>

#include "CFaziaEventSegment.h"
#include "CUdpTrigger.h"

CTCLApplication *gpTCLApplication = new faziaReadout;

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

  unsigned short port = 50000; // FAZIA's default UDP port
  std::string bindAddr;        // empty = bind to all interfaces

  if (const char *pPort = getenv("FAZIA_PORT")) {
    port = static_cast<unsigned short>(strtoul(pPort, nullptr, 0));
  }

  if (const char *pIp = getenv("FAZIA_IP")) {
    bindAddr = pIp;
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