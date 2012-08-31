//===-- MAPIPTargetMachine.cpp - Define TargetMachine for MAPIP ---------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// Top-level implementation for the MAPIP target.
//
//===----------------------------------------------------------------------===//

#include "MapipTargetMachine.h"
#include "Mapip.h"
#include "MCTargetDesc/MAPIPMCAsmInfo.h"
#include "llvm/PassManager.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

extern "C" void LLVMInitializeMapipTarget() {
  // Register the target.
  RegisterTargetMachine<MAPIPTargetMachine> X(TheMAPIPTarget);
}

MAPIPTargetMachine::MAPIPTargetMachine(const Target &T,
                                         StringRef TT,
                                         StringRef CPU,
                                         StringRef FS,
                                         const TargetOptions &Options,
                                         Reloc::Model RM, CodeModel::Model CM,
                                         CodeGenOpt::Level OL)
  : LLVMTargetMachine(T, TT, CPU, FS, Options, RM, CM, OL),
    Subtarget(TT, CPU, FS),
    // FIXME: Check TargetData string.
    DataLayout("e-p:32:32:32-i8:32:32-i16:32:32-i32:32:32-s0:32:32-n32-S32-a0:0:32"/*, 16*/),
    InstrInfo(*this), TLInfo(*this), TSInfo(*this),
    FrameLowering(Subtarget) {
}

namespace {
/// MAPIP Code Generator Pass Configuration Options.
class MAPIPPassConfig : public TargetPassConfig {
public:
  MAPIPPassConfig(MAPIPTargetMachine *TM, PassManagerBase &PM)
    : TargetPassConfig(TM, PM) {}

  MAPIPTargetMachine &getMAPIPTargetMachine() const {
    return getTM<MAPIPTargetMachine>();
  }

  virtual bool addInstSelector();
};
} // namespace

TargetPassConfig *MAPIPTargetMachine::createPassConfig(PassManagerBase &PM) {
  return new MAPIPPassConfig(this, PM);
}

bool MAPIPPassConfig::addInstSelector() {
  // Install an instruction selector.
  PM->add(createMAPIPISelDag(getMAPIPTargetMachine(), getOptLevel()));
  // Peephole optimisations
  //PM->add(createMAPIPPeephole());
  return false;
}
