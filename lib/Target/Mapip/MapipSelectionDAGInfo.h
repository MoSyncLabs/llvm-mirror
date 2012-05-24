//===-- MAPIPSelectionDAGInfo.h - MAPIP SelectionDAG Info -----*- C++ -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file defines the MAPIP subclass for TargetSelectionDAGInfo.
//
//===----------------------------------------------------------------------===//

#ifndef MAPIPSELECTIONDAGINFO_H
#define MAPIPSELECTIONDAGINFO_H

#include "llvm/Target/TargetSelectionDAGInfo.h"

namespace llvm {

class MAPIPTargetMachine;

class MAPIPSelectionDAGInfo : public TargetSelectionDAGInfo {
public:
  explicit MAPIPSelectionDAGInfo(const MAPIPTargetMachine &TM);
  ~MAPIPSelectionDAGInfo();
};

}

#endif
