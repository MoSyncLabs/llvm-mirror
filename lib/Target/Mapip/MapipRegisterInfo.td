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

def A  : MAPIPReg<0x0,  "i0">;
def B  : MAPIPReg<0x1,  "i1">;
def C  : MAPIPReg<0x2,  "i2">;
def X  : MAPIPReg<0x3,  "d0">;
def Y  : MAPIPReg<0x4,  "d1">;
def Z  : MAPIPReg<0x5,  "d2">;
def I  : MAPIPReg<0x6,  "d3">;
def J  : MAPIPReg<0x7,  "d4">; // FP if needed

def FR  : MAPIPReg<0x1c,  "fr">;
def SP  : MAPIPReg<0x1b,  "sp">;
//def EX  : MAPIPReg<0x1d,  "EX">;

// Volatile registers
def GR16 : RegisterClass<"MAPIP", [i32], 32,
  (add A,B,C,X,Y,Z,I,J,SP)>;

// Stack pointer register
def SPR16 : RegisterClass<"MAPIP", [i32], 32,
  (add SP)>;

/*
// Overflow register
def EXR16 : RegisterClass<"MAPIP", [i32], 32,
  (add EX)>;
*/

// Both volatile registers and EX
def GEXR16 : RegisterClass<"MAPIP", [i32], 32,
  (add GR16/*, EXR16*/)>;
