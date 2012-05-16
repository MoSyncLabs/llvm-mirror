//===-- MapipMCAsmInfo.cpp - Mapip asm properties -------------------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the declarations of the MapipMCAsmInfo properties.
//
//===----------------------------------------------------------------------===//

#include "MapipMCAsmInfo.h"
#include "llvm/ADT/Triple.h"

using namespace llvm;

void MapipELFMCAsmInfo::anchor() { }

MapipELFMCAsmInfo::MapipELFMCAsmInfo(const Target &T, StringRef TT) {
  //IsLittleEndian = false;
  IsLittleEndian = true;
  Triple TheTriple(TT);
  //if (TheTriple.getArch() == Triple::mapipv9)
  //  PointerSize = 8;

  Data16bitsDirective = "\t.half\t";
  Data32bitsDirective = "\t.word\t";
  Data64bitsDirective = 0;  // .xword is only supported by V9.
  ZeroDirective = "\t.space\t";
  CommentString = ";";
  //HasLEB128 = true;
  SupportsDebugInformation = true;
  
  //SunStyleELFSectionSwitchSyntax = true;
  //UsesELFSectionDirectiveForBSS = true;
  //WeakRefDirective = "\t.weak\t";
  //WeakRefDirective = "\t.extern\t";
  //PrivateGlobalPrefix = ".L";

  GlobalPrefix = "_";
  PrivateGlobalPrefix = "_";
  GlobalDirective = "\t.global\t";
  ExternDirective = "\t.extern\t";
}


