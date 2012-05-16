//===-- MapipRegisterInfo.cpp - MAPIP Register Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the MAPIP implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#include "MapipRegisterInfo.h"
#include "Mapip.h"
#include "MapipSubtarget.h"
#include "llvm/Type.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/ADT/STLExtras.h"

#define GET_REGINFO_TARGET_DESC
#include "MapipGenRegisterInfo.inc"

using namespace llvm;

MapipRegisterInfo::MapipRegisterInfo(MapipSubtarget &st,
                                     const TargetInstrInfo &tii)
  : MapipGenRegisterInfo(MAPIP::I7), Subtarget(st), TII(tii) {
}

const uint16_t* MapipRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF)
                                                                         const {
  static const uint16_t CalleeSavedRegs[] = { 0 };
  return CalleeSavedRegs;
}

BitVector MapipRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  // FIXME: G1 reserved for now for large imm generation by frame code.
  Reserved.set(MAPIP::G1);
  Reserved.set(MAPIP::G2);
  Reserved.set(MAPIP::G3);
  Reserved.set(MAPIP::G4);
  Reserved.set(MAPIP::O6);
  Reserved.set(MAPIP::I6);
  Reserved.set(MAPIP::I7);
  Reserved.set(MAPIP::G0);
  Reserved.set(MAPIP::G5);
  Reserved.set(MAPIP::G6);
  Reserved.set(MAPIP::G7);
  return Reserved;
}

void MapipRegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  MachineInstr &MI = *I;
  DebugLoc dl = MI.getDebugLoc();
  int Size = MI.getOperand(0).getImm();
  if (MI.getOpcode() == MAPIP::ADJCALLSTACKDOWN)
    Size = -Size;
  if (Size)
    BuildMI(MBB, I, dl, TII.get(MAPIP::ADDri), MAPIP::O6).addReg(MAPIP::O6).addImm(Size);
  MBB.erase(I);
}

void
MapipRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                       int SPAdj, RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  unsigned i = 0;
  MachineInstr &MI = *II;
  DebugLoc dl = MI.getDebugLoc();
  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  int FrameIndex = MI.getOperand(i).getIndex();

  // Addressable stack objects are accessed using neg. offsets from %fp
  MachineFunction &MF = *MI.getParent()->getParent();
  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex) +
               MI.getOperand(i+1).getImm();

  // Replace frame index with a frame pointer reference.
  if (Offset >= -4096 && Offset <= 4095) {
    // If the offset is small enough to fit in the immediate field, directly
    // encode it.
    MI.getOperand(i).ChangeToRegister(MAPIP::I6, false);
    MI.getOperand(i+1).ChangeToImmediate(Offset);
  } else {
    // Otherwise, emit a G1 = SETHI %hi(offset).  FIXME: it would be better to 
    // scavenge a register here instead of reserving G1 all of the time.
    unsigned OffHi = (unsigned)Offset >> 10U;
    BuildMI(*MI.getParent(), II, dl, TII.get(MAPIP::SETHIi), MAPIP::G1).addImm(OffHi);
    // Emit G1 = G1 + I6
    BuildMI(*MI.getParent(), II, dl, TII.get(MAPIP::ADDrr), MAPIP::G1).addReg(MAPIP::G1)
      .addReg(MAPIP::I6);
    // Insert: G1+%lo(offset) into the user.
    MI.getOperand(i).ChangeToRegister(MAPIP::G1, false);
    MI.getOperand(i+1).ChangeToImmediate(Offset & ((1 << 10)-1));
  }
}

void MapipRegisterInfo::
processFunctionBeforeFrameFinalized(MachineFunction &MF) const {}

unsigned MapipRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  return MAPIP::I6;
}

unsigned MapipRegisterInfo::getEHExceptionRegister() const {
  llvm_unreachable("What is the exception register");
}

unsigned MapipRegisterInfo::getEHHandlerRegister() const {
  llvm_unreachable("What is the exception handler register");
}
