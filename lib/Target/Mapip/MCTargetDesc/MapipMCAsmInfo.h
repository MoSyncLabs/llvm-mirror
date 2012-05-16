//===-- MapipMCAsmInfo.h - Mapip asm properties ----------------*- C++ -*--===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declaration of the MapipMCAsmInfo class.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPTARGETASMINFO_H
#define MAPIPTARGETASMINFO_H

#include "llvm/MC/MCAsmInfo.h"

namespace llvm {
  class StringRef;
  class Target;

  class MapipELFMCAsmInfo : public MCAsmInfo {
    virtual void anchor();
  public:
    explicit MapipELFMCAsmInfo(const Target &T, StringRef TT);
  };

} // namespace llvm

#endif
