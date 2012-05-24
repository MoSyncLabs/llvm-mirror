//===-- MAPIPSubtarget.cpp - MAPIP Subtarget Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MAPIP specific subclass of TargetSubtargetInfo.
//
//===----------------------------------------------------------------------===//

#include "MapipSubtarget.h"
#include "Mapip.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_SUBTARGETINFO_TARGET_DESC
#define GET_SUBTARGETINFO_CTOR
#include "MapipGenSubtargetInfo.inc"

using namespace llvm;

void MAPIPSubtarget::anchor() { }

MAPIPSubtarget::MAPIPSubtarget(const std::string &TT,
                                 const std::string &CPU,
                                 const std::string &FS) :
  MAPIPGenSubtargetInfo(TT, CPU, FS) {
  std::string CPUName = "generic";

  // Parse features string.
  ParseSubtargetFeatures(CPUName, FS);
}
