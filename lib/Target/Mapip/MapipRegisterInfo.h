//===-- MAPIPRegisterInfo.h - MAPIP Register Information Impl -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the MAPIP implementation of the MRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_MAPIPREGISTERINFO_H
#define LLVM_TARGET_MAPIPREGISTERINFO_H

#include "llvm/Target/TargetRegisterInfo.h"

#define GET_REGINFO_HEADER
#include "MapipGenRegisterInfo.inc"

namespace llvm {

class TargetInstrInfo;
class MAPIPTargetMachine;

struct MAPIPRegisterInfo : public MAPIPGenRegisterInfo {
private:
  MAPIPTargetMachine &TM;
  const TargetInstrInfo &TII;

  /// StackAlign - Default stack alignment.
  ///
  unsigned StackAlign;
public:
  MAPIPRegisterInfo(MAPIPTargetMachine &tm, const TargetInstrInfo &tii);

  /// Code Generation virtual methods...
  const uint16_t *getCalleeSavedRegs(const MachineFunction *MF = 0) const;
  const uint32_t *getCallPreservedMask(CallingConv::ID) const;

  BitVector getReservedRegs(const MachineFunction &MF) const;
  const TargetRegisterClass* getPointerRegClass(unsigned Kind = 0) const;

  void eliminateCallFramePseudoInstr(MachineFunction &MF,
                                     MachineBasicBlock &MBB,
                                     MachineBasicBlock::iterator I) const;

  void eliminateFrameIndex(MachineBasicBlock::iterator II,
                           int SPAdj, RegScavenger *RS = NULL) const;

  // Debug information queries.
  unsigned getFrameRegister(const MachineFunction &MF) const;
};

} // end namespace llvm

#endif // LLVM_TARGET_MAPIPREGISTERINFO_H
