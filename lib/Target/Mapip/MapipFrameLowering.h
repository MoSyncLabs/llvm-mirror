//==- MAPIPFrameLowering.h - Define frame lowering for MAPIP --*- C++ -*--==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//
//===----------------------------------------------------------------------===//

#ifndef MAPIP_FRAMEINFO_H
#define MAPIP_FRAMEINFO_H

#include "Mapip.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {
  class MAPIPSubtarget;

class MAPIPFrameLowering : public TargetFrameLowering {
protected:
  const MAPIPSubtarget &STI;

public:
  explicit MAPIPFrameLowering(const MAPIPSubtarget &sti)
    : TargetFrameLowering(TargetFrameLowering::StackGrowsDown, 4, -1, 4), STI(sti) {
  }

  /// emitProlog/emitEpilog - These methods insert prolog and epilog code into
  /// the function.
  void emitPrologue(MachineFunction &MF) const;
  void emitEpilogue(MachineFunction &MF, MachineBasicBlock &MBB) const;

  bool spillCalleeSavedRegisters(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator MI,
                                 const std::vector<CalleeSavedInfo> &CSI,
                                 const TargetRegisterInfo *TRI) const;
  bool restoreCalleeSavedRegisters(MachineBasicBlock &MBB,
                                   MachineBasicBlock::iterator MI,
                                   const std::vector<CalleeSavedInfo> &CSI,
                                   const TargetRegisterInfo *TRI) const;

  bool hasFP(const MachineFunction &MF) const;
  bool hasReservedCallFrame(const MachineFunction &MF) const;

  void processFunctionBeforeFrameFinalized(MachineFunction &MF) const;
};

} // End llvm namespace

#endif
