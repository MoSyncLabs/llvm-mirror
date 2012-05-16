//===-- Mapip.td - Describe the Mapip Target Machine -------*- tablegen -*-===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
//
//===----------------------------------------------------------------------===//

//===----------------------------------------------------------------------===//
// Target-independent interfaces which we are implementing
//===----------------------------------------------------------------------===//

include "llvm/Target/Target.td"

//===----------------------------------------------------------------------===//
// SPARC Subtarget features.
//
 
def FeatureV9
  : SubtargetFeature<"v9", "IsV9", "true",
                     "Enable SPARC-V9 instructions">;
def FeatureV8Deprecated
  : SubtargetFeature<"deprecated-v8", "V8DeprecatedInsts", "true",
                     "Enable deprecated V8 instructions in V9 mode">;
def FeatureVIS
  : SubtargetFeature<"vis", "IsVIS", "true",
                     "Enable UltraSPARC Visual Instruction Set extensions">;

//===----------------------------------------------------------------------===//
// Register File, Calling Conv, Instruction Descriptions
//===----------------------------------------------------------------------===//

include "MapipRegisterInfo.td"
include "MapipCallingConv.td"
include "MapipInstrInfo.td"

def MapipInstrInfo : InstrInfo;

//===----------------------------------------------------------------------===//
// SPARC processors supported.
//===----------------------------------------------------------------------===//

class Proc<string Name, list<SubtargetFeature> Features>
 : Processor<Name, NoItineraries, Features>;

def : Proc<"generic",         []>;
def : Proc<"v8",              []>;
def : Proc<"supermapip",      []>;
def : Proc<"mapiplite",       []>;
def : Proc<"f934",            []>;
def : Proc<"hypermapip",      []>;
def : Proc<"mapiplite86x",    []>;
def : Proc<"mapiplet",        []>;
def : Proc<"tsc701",          []>;
def : Proc<"v9",              [FeatureV9]>;
def : Proc<"ultramapip",      [FeatureV9, FeatureV8Deprecated]>;
def : Proc<"ultramapip3",     [FeatureV9, FeatureV8Deprecated]>;
def : Proc<"ultramapip3-vis", [FeatureV9, FeatureV8Deprecated, FeatureVIS]>;


//===----------------------------------------------------------------------===//
// Declare the target which we are implementing
//===----------------------------------------------------------------------===//

def Mapip : Target {
  // Pull in Instruction Info:
  let InstructionSet = MapipInstrInfo;
}
