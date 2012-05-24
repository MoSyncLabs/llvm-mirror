//===- MAPIPMachineFuctionInfo.h - MAPIP machine function info -*- C++ -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file declares MAPIP-specific per-machine-function information.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPMACHINEFUNCTIONINFO_H
#define MAPIPMACHINEFUNCTIONINFO_H

#include "llvm/CodeGen/MachineFunction.h"

namespace llvm {

/// MAPIPMachineFunctionInfo - This class is derived from MachineFunction and
/// contains private MAPIP target-specific information for each MachineFunction.
class MAPIPMachineFunctionInfo : public MachineFunctionInfo {
  virtual void anchor();

  /// CalleeSavedFrameSize - Size of the callee-saved register portion of the
  /// stack frame in bytes.
  unsigned CalleeSavedFrameSize;

  /// ReturnAddrIndex - FrameIndex for return slot.
  int ReturnAddrIndex;

public:
  MAPIPMachineFunctionInfo() : CalleeSavedFrameSize(0) {}

  explicit MAPIPMachineFunctionInfo(MachineFunction &MF)
    : CalleeSavedFrameSize(0), ReturnAddrIndex(0) {}

  unsigned getCalleeSavedFrameSize() const { return CalleeSavedFrameSize; }
  void setCalleeSavedFrameSize(unsigned bytes) { CalleeSavedFrameSize = bytes; }

  int getRAIndex() const { return ReturnAddrIndex; }
  void setRAIndex(int Index) { ReturnAddrIndex = Index; }
};

} // End llvm namespace

#endif
