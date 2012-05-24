//===-- MAPIPTargetInfo.cpp - MAPIP Target Implementation ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

#include "Mapip.h"
#include "llvm/Module.h"
#include "llvm/Support/TargetRegistry.h"
using namespace llvm;

Target llvm::TheMAPIPTarget;

extern "C" void LLVMInitializeMapipTargetInfo() { 
  RegisterTarget<Triple::mapip> 
    X(TheMAPIPTarget, "mapip", "MAPIP [experimental]");
}
