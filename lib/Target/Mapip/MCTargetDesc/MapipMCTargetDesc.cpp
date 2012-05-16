//===-- MapipMCTargetDesc.cpp - Mapip Target Descriptions -----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides Mapip specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "MapipMCTargetDesc.h"
#include "MapipMCAsmInfo.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "MapipGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MapipGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MapipGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createMapipMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMapipMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMapipMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMapipMCRegisterInfo(X, MAPIP::I7);
  return X;
}

static MCSubtargetInfo *createMapipMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                   StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitMapipMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createMapipMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                               CodeModel::Model CM,
                                               CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

extern "C" void LLVMInitializeMapipTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<MapipELFMCAsmInfo> X(TheMapipTarget);
  //RegisterMCAsmInfo<MapipELFMCAsmInfo> Y(TheMapipV9Target);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheMapipTarget,
                                       createMapipMCCodeGenInfo);
  //TargetRegistry::RegisterMCCodeGenInfo(TheMapipV9Target,
  //                                     createMapipMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheMapipTarget, createMapipMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheMapipTarget, createMapipMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheMapipTarget,
                                          createMapipMCSubtargetInfo);
}
