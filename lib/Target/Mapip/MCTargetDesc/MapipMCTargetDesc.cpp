//===-- MAPIPMCTargetDesc.cpp - MAPIP Target Descriptions ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file provides MAPIP specific target descriptions.
//
//===----------------------------------------------------------------------===//

#include "MapipMCTargetDesc.h"
#include "MapipMCAsmInfo.h"
#include "InstPrinter/MapipInstPrinter.h"
#include "llvm/MC/MCCodeGenInfo.h"
#include "llvm/MC/MCInstrInfo.h"
#include "llvm/MC/MCRegisterInfo.h"
#include "llvm/MC/MCSubtargetInfo.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_MC_DESC
#include "MapipGenInstrInfo.inc"

#define GET_SUBTARGETINFO_MC_DESC
#include "MapipGenSubtargetInfo.inc"

#define GET_REGINFO_MC_DESC
#include "MapipGenRegisterInfo.inc"

using namespace llvm;

static MCInstrInfo *createMAPIPMCInstrInfo() {
  MCInstrInfo *X = new MCInstrInfo();
  InitMAPIPMCInstrInfo(X);
  return X;
}

static MCRegisterInfo *createMAPIPMCRegisterInfo(StringRef TT) {
  MCRegisterInfo *X = new MCRegisterInfo();
  InitMAPIPMCRegisterInfo(X, MAPIP::A);
  return X;
}

static MCSubtargetInfo *createMAPIPMCSubtargetInfo(StringRef TT, StringRef CPU,
                                                    StringRef FS) {
  MCSubtargetInfo *X = new MCSubtargetInfo();
  InitMAPIPMCSubtargetInfo(X, TT, CPU, FS);
  return X;
}

static MCCodeGenInfo *createMAPIPMCCodeGenInfo(StringRef TT, Reloc::Model RM,
                                                CodeModel::Model CM,
                                                CodeGenOpt::Level OL) {
  MCCodeGenInfo *X = new MCCodeGenInfo();
  X->InitMCCodeGenInfo(RM, CM, OL);
  return X;
}

static MCInstPrinter *createMAPIPMCInstPrinter(const Target &T,
                                                unsigned SyntaxVariant,
                                                const MCAsmInfo &MAI,
                                                const MCInstrInfo &MII,
                                                const MCRegisterInfo &MRI,
                                                const MCSubtargetInfo &STI) {
  if (SyntaxVariant == 0)
    return new MAPIPInstPrinter(MAI, MII, MRI);
  return 0;
}

extern "C" void LLVMInitializeMapipTargetMC() {
  // Register the MC asm info.
  RegisterMCAsmInfo<MAPIPMCAsmInfo> X(TheMAPIPTarget);

  // Register the MC codegen info.
  TargetRegistry::RegisterMCCodeGenInfo(TheMAPIPTarget,
                                        createMAPIPMCCodeGenInfo);

  // Register the MC instruction info.
  TargetRegistry::RegisterMCInstrInfo(TheMAPIPTarget, createMAPIPMCInstrInfo);

  // Register the MC register info.
  TargetRegistry::RegisterMCRegInfo(TheMAPIPTarget,
                                    createMAPIPMCRegisterInfo);

  // Register the MC subtarget info.
  TargetRegistry::RegisterMCSubtargetInfo(TheMAPIPTarget,
                                          createMAPIPMCSubtargetInfo);

  // Register the MCInstPrinter.
  TargetRegistry::RegisterMCInstPrinter(TheMAPIPTarget,
                                        createMAPIPMCInstPrinter);
}
