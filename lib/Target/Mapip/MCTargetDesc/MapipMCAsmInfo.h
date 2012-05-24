//===-- MAPIPMCAsmInfo.h - MAPIP asm properties --------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MAPIPMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPTARGETASMINFO_H
#define MAPIPTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class StringRef;
  class Target;

  class MAPIPMCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit MAPIPMCAsmInfo(const Target &T, StringRef TT);
  };

} // namespace llvm

#endif
