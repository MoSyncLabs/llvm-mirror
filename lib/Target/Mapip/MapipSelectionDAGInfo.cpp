//===-- MAPIPSelectionDAGInfo.cpp - MAPIP SelectionDAG Info -------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file implements the MAPIPSelectionDAGInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mapip-selectiondag-info"
#include "MapipTargetMachine.h"
using namespace llvm;

MAPIPSelectionDAGInfo::MAPIPSelectionDAGInfo(const MAPIPTargetMachine &TM)
  : TargetSelectionDAGInfo(TM) {
}

MAPIPSelectionDAGInfo::~MAPIPSelectionDAGInfo() {
}
