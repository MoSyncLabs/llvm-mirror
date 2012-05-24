//==-- MAPIP.h - Top-level interface for MAPIP representation --*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the entry points for global functions defined in
// the LLVM MAPIP backend.
//
//===----------------------------------------------------------------------===//

#ifndef LLVM_TARGET_MAPIP_H
#define LLVM_TARGET_MAPIP_H

#include "MCTargetDesc/MapipMCTargetDesc.h"
#include "llvm/Target/TargetMachine.h"

namespace MAPIPCC {
  // MAPIP specific condition code.
  enum CondCodes {
    COND_B,  // a&b != 0
    COND_C,  // a&b == 0
    COND_E,  // ==
    COND_NE, // !=

    COND_G,  // u>
    COND_A,  // s>
    COND_L,  // u<
    COND_U,  // s<

    // These are pseudo CCs, they get expanded to two IF_ instructions
    COND_GE, // u>=
    COND_AE, // s>=
    COND_LE, // u<=
    COND_UE, // s<=

    COND_INVALID = -1
  };
}

namespace llvm {
  class MAPIPTargetMachine;
  class FunctionPass;
  class formatted_raw_ostream;

  FunctionPass *createMAPIPISelDag(MAPIPTargetMachine &TM, CodeGenOpt::Level OptLevel);
  FunctionPass *createMAPIPPeephole();

} // end namespace llvm;

#endif
