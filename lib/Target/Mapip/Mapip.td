//===-- MAPIP.td - Describe the MAPIP Target Machine -----*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source 
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
// This is the top level entry point for the MAPIP target.
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// Subtarget Features. 
//===----------------------------------------------------------------------===//
def FeatureX
 : SubtargetFeature<"ext", "ExtendedInsts", "true",
                    "Enable MAPIP-X extensions">;

//===----------------------------------------------------------------------===//
// MAPIP supported processors.
//===----------------------------------------------------------------------===//
class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic",         []>;

//===----------------------------------------------------------------------===//
// Register File Description
//===----------------------------------------------------------------------===//

include "MapipRegisterInfo.td"

//===----------------------------------------------------------------------===//
// Calling Convention Description
//===----------------------------------------------------------------------===//

include "MapipCallingConv.td"

//===----------------------------------------------------------------------===//
// Instruction Descriptions
//===----------------------------------------------------------------------===//

include "MapipInstrInfo.td"

def MAPIPInstrInfo : InstrInfo;

def MAPIPInstPrinter : AsmWriter {
  string AsmWriterClassName  = "InstPrinter";
  bit isMCAsmWriter = 1;
}

//===----------------------------------------------------------------------===//
// Target Declaration
//===----------------------------------------------------------------------===//

def MAPIP : Target {
  let InstructionSet = MAPIPInstrInfo;
  let AssemblyWriters = [MAPIPInstPrinter];
}

