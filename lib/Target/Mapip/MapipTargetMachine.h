//===-- MAPIPTargetMachine.h - Define TargetMachine for MAPIP -*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the MAPIP specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//


#ifndef LLVM_TARGET_MAPIP_TARGETMACHINE_H
#define LLVM_TARGET_MAPIP_TARGETMACHINE_H

#include "MapipInstrInfo.h"
#include "MapipISelLowering.h"
#include "MapipFrameLowering.h"
#include "MapipSelectionDAGInfo.h"
#include "MapipRegisterInfo.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"
#include "llvm/Target/TargetMachine.h"

namespace llvm {

/// MAPIPTargetMachine
///
class MAPIPTargetMachine : public LLVMTargetMachine {
  MAPIPSubtarget        Subtarget;
  const TargetData       DataLayout;       // Calculates type size & alignment
  MAPIPInstrInfo        InstrInfo;
  MAPIPTargetLowering   TLInfo;
  MAPIPSelectionDAGInfo TSInfo;
  MAPIPFrameLowering    FrameLowering;

public:
  MAPIPTargetMachine(const Target &T, StringRef TT,
                      StringRef CPU, StringRef FS, const TargetOptions &Options,
                      Reloc::Model RM, CodeModel::Model CM,
                      CodeGenOpt::Level OL);

  virtual const TargetFrameLowering *getFrameLowering() const {
    return &FrameLowering;
  }
  virtual const MAPIPInstrInfo *getInstrInfo() const  { return &InstrInfo; }
  virtual const TargetData *getTargetData() const     { return &DataLayout;}
  virtual const MAPIPSubtarget *getSubtargetImpl() const { return &Subtarget; }

  virtual const TargetRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }

  virtual const MAPIPTargetLowering *getTargetLowering() const {
    return &TLInfo;
  }

  virtual const MAPIPSelectionDAGInfo* getSelectionDAGInfo() const {
    return &TSInfo;
  }

  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
}; // MAPIPTargetMachine.

} // end namespace llvm

#endif // LLVM_TARGET_MAPIP_TARGETMACHINE_H
