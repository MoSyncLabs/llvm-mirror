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
    MCOND_EQ,  // a==b
    MCOND_NE,  // a!=b
    MCOND_GE,  // a>=b
    MCOND_GEU, // (unsigned)a>=(unsigned)b
    MCOND_GT,  // a>b
    MCOND_GTU,  // (unsigned)a>(unsigned)b
    MCOND_LE,  // a<=b
    MCOND_LEU,  // (unsigned)a<=(unsigned)b
    MCOND_LT, // a<b
    MCOND_LTU, // (unsigned)a<(unsigned)b

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
