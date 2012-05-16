//===-- MapipFrameLowering.cpp - Mapip Frame Information ------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of TargetFrameLowering class.
//
//===----------------------------------------------------------------------===//

#include "MapipFrameLowering.h"
#include "MapipInstrInfo.h"
#include "MapipMachineFunctionInfo.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/Support/CommandLine.h"

using namespace llvm;

void MapipFrameLowering::emitPrologue(MachineFunction &MF) const {
  MachineBasicBlock &MBB = MF.front();
  MachineFrameInfo *MFI = MF.getFrameInfo();
  const MapipInstrInfo &TII =
    *static_cast<const MapipInstrInfo*>(MF.getTarget().getInstrInfo());
  MachineBasicBlock::iterator MBBI = MBB.begin();
  DebugLoc dl = MBBI != MBB.end() ? MBBI->getDebugLoc() : DebugLoc();

  // Get the number of bytes to allocate from the FrameInfo
  int NumBytes = (int) MFI->getStackSize();

  // Emit the correct save instruction based on the number of bytes in
  // the frame. Minimum stack frame size according to V8 ABI is:
  //   16 words for register window spill
  //    1 word for address of returned aggregate-value
  // +  6 words for passing parameters on the stack
  // ----------
  //   23 words * 4 bytes per word = 92 bytes
  NumBytes += 92;

  // Round up to next doubleword boundary -- a double-word boundary
  // is required by the ABI.
  NumBytes = (NumBytes + 7) & ~7;
  NumBytes = -NumBytes;

  if (NumBytes >= -4096) {
    BuildMI(MBB, MBBI, dl, TII.get(MAPIP::SAVEri), MAPIP::O6)
      .addReg(MAPIP::O6).addImm(NumBytes);
  } else {
    // Emit this the hard way.  This clobbers G1 which we always know is
    // available here.
    unsigned OffHi = (unsigned)NumBytes >> 10U;
    BuildMI(MBB, MBBI, dl, TII.get(MAPIP::SETHIi), MAPIP::G1).addImm(OffHi);
    // Emit G1 = G1 + I6
    BuildMI(MBB, MBBI, dl, TII.get(MAPIP::ORri), MAPIP::G1)
      .addReg(MAPIP::G1).addImm(NumBytes & ((1 << 10)-1));
    BuildMI(MBB, MBBI, dl, TII.get(MAPIP::SAVErr), MAPIP::O6)
      .addReg(MAPIP::O6).addReg(MAPIP::G1);
  }
}

void MapipFrameLowering::emitEpilogue(MachineFunction &MF,
                                  MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator MBBI = MBB.getLastNonDebugInstr();
  const MapipInstrInfo &TII =
    *static_cast<const MapipInstrInfo*>(MF.getTarget().getInstrInfo());
  DebugLoc dl = MBBI->getDebugLoc();
  assert(MBBI->getOpcode() == MAPIP::RETL &&
         "Can only put epilog before 'retl' instruction!");
  BuildMI(MBB, MBBI, dl, TII.get(MAPIP::RESTORErr), MAPIP::G0).addReg(MAPIP::G0)
    .addReg(MAPIP::G0);
}
