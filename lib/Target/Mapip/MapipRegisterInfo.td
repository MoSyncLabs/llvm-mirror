//===-- MAPIPRegisterInfo.td - MAPIP Register defs -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
//  Declarations that describe the MAPIP register file
//===----------------------------------------------------------------------===//

class MAPIPReg<bits<5> num, string n> : Register<n> {
  field bits<5> Num = num;
  let Namespace = "MAPIP";
}

//===----------------------------------------------------------------------===//
//  Registers
//===----------------------------------------------------------------------===//

def A  : MAPIPReg<0x0,  "A">;
def B  : MAPIPReg<0x1,  "B">;
def C  : MAPIPReg<0x2,  "C">;
def X  : MAPIPReg<0x3,  "X">;
def Y  : MAPIPReg<0x4,  "Y">;
def Z  : MAPIPReg<0x5,  "Z">;
def I  : MAPIPReg<0x6,  "I">;
def J  : MAPIPReg<0x7,  "J">; // FP if needed

def SP  : MAPIPReg<0x1b,  "SP">;
def EX  : MAPIPReg<0x1d,  "EX">;

// Volatile registers
def GR16 : RegisterClass<"MAPIP", [i32], 32,
  (add A,B,C,X,Y,Z,I,J,SP)>;

// Stack pointer register
def SPR16 : RegisterClass<"MAPIP", [i32], 32,
  (add SP)>;

// Overflow register
def EXR16 : RegisterClass<"MAPIP", [i32], 32,
  (add EX)>;

// Both volatile registers and EX
def GEXR16 : RegisterClass<"MAPIP", [i32], 32,
  (add GR16, EXR16)>;
