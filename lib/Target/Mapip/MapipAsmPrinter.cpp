//===-- MAPIPAsmPrinter.cpp - MAPIP LLVM assembly writer ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains a printer that converts from our internal representation
// of machine-dependent LLVM code to the MAPIP assembly language.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "asm-printer"
#include "Mapip.h"
#include "MapipInstrInfo.h"
#include "MapipMCInstLower.h"
#include "MapipTargetMachine.h"
#include "InstPrinter/MAPIPInstPrinter.h"
#include "llvm/Constants.h"
#include "llvm/DerivedTypes.h"
#include "llvm/Module.h"
#include "llvm/Assembly/Writer.h"
#include "llvm/CodeGen/AsmPrinter.h"
#include "llvm/CodeGen/MachineModuleInfo.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineConstantPool.h"
#include "llvm/CodeGen/MachineInstr.h"
#include "llvm/MC/MCAsmInfo.h"
#include "llvm/MC/MCInst.h"
#include "llvm/MC/MCStreamer.h"
#include "llvm/MC/MCSymbol.h"
#include "llvm/Target/Mangler.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/ADT/SmallString.h"

using namespace llvm;

namespace {
  class MAPIPAsmPrinter : public AsmPrinter {
  public:
    MAPIPAsmPrinter(TargetMachine &TM, MCStreamer &Streamer)
      : AsmPrinter(TM, Streamer) {}

    virtual const char *getPassName() const {
      return "MAPIP Assembly Printer";
    }

    void printOperand(const MachineInstr *MI, int OpNum,
                      raw_ostream &O, const char* Modifier = 0);
    void printSrcMemOperand(const MachineInstr *MI, int OpNum,
                            raw_ostream &O);
    bool PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                         unsigned AsmVariant, const char *ExtraCode,
                         raw_ostream &O);
    bool PrintAsmMemoryOperand(const MachineInstr *MI,
                               unsigned OpNo, unsigned AsmVariant,
                               const char *ExtraCode, raw_ostream &O);
    void EmitInstruction(const MachineInstr *MI);
    void EmitFunctionEntryLabel();
  };
} // end of anonymous namespace


void MAPIPAsmPrinter::printOperand(const MachineInstr *MI, int OpNum,
                                    raw_ostream &O, const char *Modifier) {
  const MachineOperand &MO = MI->getOperand(OpNum);
  switch (MO.getType()) {
  default: llvm_unreachable("Not implemented yet!");
  case MachineOperand::MO_Register:
    O << MAPIPInstPrinter::getRegisterName(MO.getReg());
    return;
  case MachineOperand::MO_Immediate:
    O << MO.getImm();
    return;
  case MachineOperand::MO_MachineBasicBlock:
    O << *MO.getMBB()->getSymbol();
    return;
  case MachineOperand::MO_GlobalAddress: {
    bool isMemOp  = Modifier && !strcmp(Modifier, "mem");
//    uint64_t Offset = MO.getOffset();
    int64_t Offset = MO.getOffset();

    if (isMemOp) O << '[';
    O << *Mang->getSymbol(MO.getGlobal());
    if (Offset)
//      O << '+' << Offset;
        O << ',' << Offset;
    if (isMemOp) O << ']';

    return;
  }
  case MachineOperand::MO_ExternalSymbol: {
    bool isMemOp  = Modifier && !strcmp(Modifier, "mem");
    if (isMemOp) O << '[';
    O << MAI->getGlobalPrefix() << MO.getSymbolName();
    if (isMemOp) O << ']';
    return;
  }
  }
}

void MAPIPAsmPrinter::printSrcMemOperand(const MachineInstr *MI, int OpNum,
                                          raw_ostream &O) {
  const MachineOperand &Base = MI->getOperand(OpNum);
  const MachineOperand &Disp = MI->getOperand(OpNum+1);

  // Special case for PICK n syntax
/*
  if (Base.getReg() == MAPIP::SP) {
    if (Disp.isImm()) {
      if (Disp.getImm() == 0) {
        O << "PEEK";  // equiv. to [SP]
      } else {
        O << "PICK 0x"; // equiv. to [SP+x]
        O.write_hex(Disp.getImm() & 0xFFFF);
      }
    } else {
      llvm_unreachable("Unsupported src mem expression in inline asm");
    }
    return;
  }
*/

  O << '[';

  if (Base.getReg()) {
    O << MAPIPInstPrinter::getRegisterName(Base.getReg());
  }

  if (Base.getReg()) {
    // Only print the immediate if it isn't 0, easier to read and
    // generates more efficient code on bad assemblers
    if (Disp.getImm() != 0) {
//      O << "+";
        O << ",";

      O << "0x";
      //O.write_hex(Disp.getImm() & 0xFFFF);
      O.write_hex(Disp.getImm());
    }
  } else {
    O << "0x";
//    O.write_hex(Disp.getImm() & 0xFFFF);
      O.write_hex(Disp.getImm());
  }

  O << ']';
}

/// PrintAsmOperand - Print out an operand for an inline asm expression.
///
bool MAPIPAsmPrinter::PrintAsmOperand(const MachineInstr *MI, unsigned OpNo,
                                       unsigned AsmVariant,
                                       const char *ExtraCode, raw_ostream &O) {
  // Does this asm operand have a single letter operand modifier?
  if (ExtraCode && ExtraCode[0])
    return true; // Unknown modifier.

  printOperand(MI, OpNo, O);
  return false;
}

bool MAPIPAsmPrinter::PrintAsmMemoryOperand(const MachineInstr *MI,
                                             unsigned OpNo, unsigned AsmVariant,
                                             const char *ExtraCode,
                                             raw_ostream &O) {
  if (ExtraCode && ExtraCode[0]) {
    return true; // Unknown modifier.
  }
  printSrcMemOperand(MI, OpNo, O);
  return false;
}

//===----------------------------------------------------------------------===//
void MAPIPAsmPrinter::EmitInstruction(const MachineInstr *MI) {
  MAPIPMCInstLower MCInstLowering(OutContext, *Mang, *this);

  MCInst TmpInst;
  MCInstLowering.Lower(MI, TmpInst);
  OutStreamer.EmitInstruction(TmpInst);
}

void MAPIPAsmPrinter::EmitFunctionEntryLabel() {
   std::string Str;
   raw_string_ostream O(Str);

   // The function label could have already been emitted if two symbols end up
   // conflicting due to asm renaming.  Detect this and emit an error.
   if (CurrentFnSym->isUndefined())
   {
     const Function* function = MF->getFunction();
     Type* returnType = function->getReturnType();
     const Function::ArgumentListType& arguments = function->getArgumentList();

     O << ".func " << CurrentFnSym->getName() << ", " << arguments.size() << ", ";
     if(returnType->isIntegerTy())
         O << "int";
     else if(returnType->isFloatTy())
         O << "float";
     else if(returnType->isDoubleTy())
         O << "double";
     else if(returnType->isVoidTy())
         O << "void";
     else
         report_fatal_error("'" + Twine(CurrentFnSym->getName()) +
                      "' invalid return type.");

     OutStreamer.EmitRawText((StringRef)O.str());
     return;
   }

   report_fatal_error("'" + Twine(CurrentFnSym->getName()) +
                      "' label emitted multiple times to assembly file");
}

// Force static initialization.
extern "C" void LLVMInitializeMapipAsmPrinter() {
  RegisterAsmPrinter<MAPIPAsmPrinter> X(TheMAPIPTarget);
}
