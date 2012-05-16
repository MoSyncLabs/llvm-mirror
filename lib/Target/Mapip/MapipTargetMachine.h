//===-- MapipTargetMachine.h - Define TargetMachine for Mapip ---*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares the Mapip specific subclass of TargetMachine.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPTARGETMACHINE_H
#define MAPIPTARGETMACHINE_H

#include "MapipInstrInfo.h"
#include "MapipISelLowering.h"
#include "MapipFrameLowering.h"
#include "MapipSelectionDAGInfo.h"
#include "MapipSubtarget.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetData.h"
#include "llvm/Target/TargetFrameLowering.h"

namespace llvm {

class MapipTargetMachine : public LLVMTargetMachine {
  MapipSubtarget Subtarget;
  const TargetData DataLayout;       // Calculates type size & alignment
  MapipInstrInfo InstrInfo;
  MapipTargetLowering TLInfo;
  MapipSelectionDAGInfo TSInfo;
  MapipFrameLowering FrameLowering;
public:
  MapipTargetMachine(const Target &T, StringRef TT,
                     StringRef CPU, StringRef FS, const TargetOptions &Options,
                     Reloc::Model RM, CodeModel::Model CM,
                     CodeGenOpt::Level OL, bool is64bit);

  virtual const MapipInstrInfo *getInstrInfo() const { return &InstrInfo; }
  virtual const TargetFrameLowering  *getFrameLowering() const {
    return &FrameLowering;
  }
  virtual const MapipSubtarget   *getSubtargetImpl() const{ return &Subtarget; }
  virtual const MapipRegisterInfo *getRegisterInfo() const {
    return &InstrInfo.getRegisterInfo();
  }
  virtual const MapipTargetLowering* getTargetLowering() const {
    return &TLInfo;
  }
  virtual const MapipSelectionDAGInfo* getSelectionDAGInfo() const {
    return &TSInfo;
  }
  virtual const TargetData       *getTargetData() const { return &DataLayout; }

  // Pass Pipeline Configuration
  virtual TargetPassConfig *createPassConfig(PassManagerBase &PM);
};

/// MapipV8TargetMachine - Mapip 32-bit target machine
///
class MapipV8TargetMachine : public MapipTargetMachine {
  virtual void anchor();
public:
  MapipV8TargetMachine(const Target &T, StringRef TT,
                       StringRef CPU, StringRef FS,
                       const TargetOptions &Options,
                       Reloc::Model RM, CodeModel::Model CM,
                       CodeGenOpt::Level OL);
};

/// MapipV9TargetMachine - Mapip 64-bit target machine
///
class MapipV9TargetMachine : public MapipTargetMachine {
  virtual void anchor();
public:
  MapipV9TargetMachine(const Target &T, StringRef TT,
                       StringRef CPU, StringRef FS,
                       const TargetOptions &Options,
                       Reloc::Model RM, CodeModel::Model CM,
                       CodeGenOpt::Level OL);
};

} // end namespace llvm

#endif
