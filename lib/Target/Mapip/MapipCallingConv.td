//==- MAPIPCallingConv.td - Calling Conventions for MAPIP -*- tablegen -*-==//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This describes the calling conventions for MAPIP architecture.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// MAPIP Return Value Calling Convention
//===----------------------------------------------------------------------===//
def RetCC_MAPIP : CallingConv<[
  // Promote i8, i16 return values to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // i16 are returned in registers A, B, C
  CCIfType<[i32], CCAssignToReg<[A, B, C]>>
]>;

//===----------------------------------------------------------------------===//
// MAPIP Argument Calling Conventions
//===----------------------------------------------------------------------===//
def CC_MAPIP : CallingConv<[
  // Promote i8, i16 return values to i32.
  CCIfType<[i8, i16], CCPromoteToType<i32>>,

  // The first 3 integer arguments of non-varargs functions are passed in
  // integer registers.
  CCIfNotVarArg<CCIfType<[i32], CCAssignToReg<[A, B, C]>>>,

  // Integer values get stored in stack slots that are 1 (16 bit) byte in
  // size and 1-byte aligned.
  CCIfType<[i32], CCAssignToStack<4, 4>>
]>;

def CSR_MAPIP : CalleeSavedRegs<(add X, Y, Z, I, J)>;
