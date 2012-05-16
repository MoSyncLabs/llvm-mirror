//===-- FPMover.cpp - Mapip double-precision floating point move fixer ----===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Expand FpMOVD/FpABSD/FpNEGD instructions into their single-precision pieces.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "fpmover"
#include "Mapip.h"
#include "MapipSubtarget.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/raw_ostream.h"
using namespace llvm;

STATISTIC(NumFpDs , "Number of instructions translated");
STATISTIC(NoopFpDs, "Number of noop instructions removed");

namespace {
  struct FPMover : public MachineFunctionPass {
    /// Target machine description which we query for reg. names, data
    /// layout, etc.
    ///
    TargetMachine &TM;
    
    static char ID;
    explicit FPMover(TargetMachine &tm) 
      : MachineFunctionPass(ID), TM(tm) { }

    virtual const char *getPassName() const {
      return "Mapip Double-FP Move Fixer";
    }

    bool runOnMachineBasicBlock(MachineBasicBlock &MBB);
    bool runOnMachineFunction(MachineFunction &F);
  };
  char FPMover::ID = 0;
} // end of anonymous namespace

/// createMapipFPMoverPass - Returns a pass that turns FpMOVD
/// instructions into FMOVS instructions
///
FunctionPass *llvm::createMapipFPMoverPass(TargetMachine &tm) {
  return new FPMover(tm);
}

/// getDoubleRegPair - Given a DFP register, return the even and odd FP
/// registers that correspond to it.
static void getDoubleRegPair(unsigned DoubleReg, unsigned &EvenReg,
                             unsigned &OddReg) {
  static const uint16_t EvenHalvesOfPairs[] = {
    MAPIP::F0, MAPIP::F2, MAPIP::F4, MAPIP::F6, MAPIP::F8, MAPIP::F10, MAPIP::F12, MAPIP::F14,
    MAPIP::F16, MAPIP::F18, MAPIP::F20, MAPIP::F22, MAPIP::F24, MAPIP::F26, MAPIP::F28, MAPIP::F30
  };
  static const uint16_t OddHalvesOfPairs[] = {
    MAPIP::F1, MAPIP::F3, MAPIP::F5, MAPIP::F7, MAPIP::F9, MAPIP::F11, MAPIP::F13, MAPIP::F15,
    MAPIP::F17, MAPIP::F19, MAPIP::F21, MAPIP::F23, MAPIP::F25, MAPIP::F27, MAPIP::F29, MAPIP::F31
  };
  static const uint16_t DoubleRegsInOrder[] = {
    MAPIP::D0, MAPIP::D1, MAPIP::D2, MAPIP::D3, MAPIP::D4, MAPIP::D5, MAPIP::D6, MAPIP::D7, MAPIP::D8,
    MAPIP::D9, MAPIP::D10, MAPIP::D11, MAPIP::D12, MAPIP::D13, MAPIP::D14, MAPIP::D15
  };
  for (unsigned i = 0; i < array_lengthof(DoubleRegsInOrder); ++i)
    if (DoubleRegsInOrder[i] == DoubleReg) {
      EvenReg = EvenHalvesOfPairs[i];
      OddReg = OddHalvesOfPairs[i];
      return;
    }
  llvm_unreachable("Can't find reg");
}

/// runOnMachineBasicBlock - Fixup FpMOVD instructions in this MBB.
///
bool FPMover::runOnMachineBasicBlock(MachineBasicBlock &MBB) {
  bool Changed = false;
  for (MachineBasicBlock::iterator I = MBB.begin(); I != MBB.end(); ) {
    MachineInstr *MI = I++;
    DebugLoc dl = MI->getDebugLoc();
    if (MI->getOpcode() == MAPIP::FpMOVD || MI->getOpcode() == MAPIP::FpABSD ||
        MI->getOpcode() == MAPIP::FpNEGD) {
      Changed = true;
      unsigned DestDReg = MI->getOperand(0).getReg();
      unsigned SrcDReg  = MI->getOperand(1).getReg();
      if (DestDReg == SrcDReg && MI->getOpcode() == MAPIP::FpMOVD) {
        MBB.erase(MI);   // Eliminate the noop copy.
        ++NoopFpDs;
        continue;
      }
      
      unsigned EvenSrcReg = 0, OddSrcReg = 0, EvenDestReg = 0, OddDestReg = 0;
      getDoubleRegPair(DestDReg, EvenDestReg, OddDestReg);
      getDoubleRegPair(SrcDReg, EvenSrcReg, OddSrcReg);

      const TargetInstrInfo *TII = TM.getInstrInfo();
      if (MI->getOpcode() == MAPIP::FpMOVD)
        MI->setDesc(TII->get(MAPIP::FMOVS));
      else if (MI->getOpcode() == MAPIP::FpNEGD)
        MI->setDesc(TII->get(MAPIP::FNEGS));
      else if (MI->getOpcode() == MAPIP::FpABSD)
        MI->setDesc(TII->get(MAPIP::FABSS));
      else
        llvm_unreachable("Unknown opcode!");
        
      MI->getOperand(0).setReg(EvenDestReg);
      MI->getOperand(1).setReg(EvenSrcReg);
      DEBUG(errs() << "FPMover: the modified instr is: " << *MI);
      // Insert copy for the other half of the double.
      if (DestDReg != SrcDReg) {
        MI = BuildMI(MBB, I, dl, TM.getInstrInfo()->get(MAPIP::FMOVS), OddDestReg)
          .addReg(OddSrcReg);
        DEBUG(errs() << "FPMover: the inserted instr is: " << *MI);
      }
      ++NumFpDs;
    }
  }
  return Changed;
}

bool FPMover::runOnMachineFunction(MachineFunction &F) {
  // If the target has V9 instructions, the fp-mover pseudos will never be
  // emitted.  Avoid a scan of the instructions to improve compile time.
  if (TM.getSubtarget<MapipSubtarget>().isV9())
    return false;
  
  bool Changed = false;
  for (MachineFunction::iterator FI = F.begin(), FE = F.end();
       FI != FE; ++FI)
    Changed |= runOnMachineBasicBlock(*FI);
  return Changed;
}
