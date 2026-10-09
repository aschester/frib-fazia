/**
 * @file faziaReadout.h
 * @brief This file contains the definition of the faziaReadout class, which
 * implements the readout for the FAZIA detector based on the SBS readout
 * framework.
 */

#ifndef FAZIAREADOUT_H
#define FAZIAREADOUT_H

#include <CReadoutMain.h>

class CTclInterpreter;
class CExperiment;

namespace fazia {

/**
 * @class faziaReadout
 * @brief This class implements the readout for the FAZIA detector based on the
 * SBS readout framework.
 */

class faziaReadout : public CReadoutMain {
public:
  /**
   * @brief Setup the Readout.
   * @param pExperiment Pointer to the experiment object.
   */
  virtual void SetupReadout(CExperiment *pExperiment);
  /**
   * @brief Setup the scaler Readout.
   * @param pExperiment Pointer to the experiment object.
   */
  virtual void SetupScalers(CExperiment *pExperiment);
  /**
   * @brief Used to add Tcl commands. See the CTCLObjectProcessor class.
   * @param pInterp Pointer to CTCLInterpreter object that encapsulates the
   * Tcl_Interp* of our main interpreter.
   */
  virtual void addCommands(CTCLInterpreter *pInterp);
  /**
   * @brief Setup run variables.
   * @param pInterp Pointer to CTCLInterpreter object that encapsulates the
   * Tcl_Interp* of our main interpreter.
   */
  virtual void SetupRunVariables(CTCLInterpreter *pInterp);
  /**
   * @brief Setup state variables.
   * @param pInterp Pointer to CTCLInterpreter object that encapsulates the
   * Tcl_Interp* of our main interpreter.
   */
  virtual void SetupStateVariables(CTCLInterpreter *pInterp);
};

} // namespace fazia

#endif