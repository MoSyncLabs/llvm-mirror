//===-- MAPIPMCAsmInfo.cpp - MAPIP asm properties -----------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the MAPIPMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "MapipMCAsmInfo.h"
#include "llvm/ADT/StringRef.h"
using namespace llvm;

void MAPIPMCAsmInfo::anchor() { }

MAPIPMCAsmInfo::MAPIPMCAsmInfo(const Target &T, StringRef TT) {
  LabelSuffix = ":";
  PointerSize = 4;

  PrivateGlobalPrefix = "L";
  GlobalPrefix = "_";
  GlobalDirective = "\t.global\t";
  WeakRefDirective ="\t.weak\t";
  PCSymbol=".";
  CommentString = ";";
  AscizDirective = NULL;
  IsLittleEndian = true;

  //LCOMMDirectiveType = LCOMM::None;

  AlignmentIsInBytes = true;
  AlignDirective = "\t.align\t";
  AllowNameToStartWithDigit = true;
  UsesELFSectionDirectiveForBSS = false;
  HasDotTypeDotSizeDirective = false;
  HasSingleParameterDotFile = false;
}
