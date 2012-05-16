//===-- MapipTargetInfo.cpp - Mapip Target Implementation -----------------===//
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

Target llvm::TheMapipTarget;
//Target llvm::TheMapipV9Target;

extern "C" void LLVMInitializeMapipTargetInfo() { 
  RegisterTarget<Triple::mapip> X(TheMapipTarget, "mapip", "Mapip");
  //RegisterTarget<Triple::mapipv9> Y(TheMapipV9Target, "mapipv9", "Mapip V9");
}
