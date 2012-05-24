//===-- MAPIPInstrInfo.cpp - MAPIP Instruction Information --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the MAPIP implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "MapipInstrInfo.h"
#include "Mapip.h"
#include "MapipMachineFunctionInfo.h"
#include "MapipTargetMachine.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"

#define GET_INSTRINFO_CTOR
#include "MAPIPGenInstrInfo.inc"

using namespace llvm;

MAPIPInstrInfo::MAPIPInstrInfo(MAPIPTargetMachine &tm)
  : MAPIPGenInstrInfo(MAPIP::ADJCALLSTACKDOWN, MAPIP::ADJCALLSTACKUP),
    RI(tm, *this), TM(tm) {}

void MAPIPInstrInfo::storeRegToStackSlot(MachineBasicBlock &MBB,
                                          MachineBasicBlock::iterator MI,
                                    unsigned SrcReg, bool isKill, int FrameIdx,
                                          const TargetRegisterClass *RC,
                                          const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();

  MachineMemOperand *MMO =
    MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FrameIdx),
                            MachineMemOperand::MOStore,
                            MFI.getObjectSize(FrameIdx),
                            MFI.getObjectAlignment(FrameIdx));

  if (RC == &MAPIP::GR16RegClass || RC == &MAPIP::GEXR16RegClass)
    BuildMI(MBB, MI, DL, get(MAPIP::MOV16mr))
      .addFrameIndex(FrameIdx).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill)).addMemOperand(MMO);
  else
    llvm_unreachable("Cannot store this register to stack slot!");
}

void MAPIPInstrInfo::loadRegFromStackSlot(MachineBasicBlock &MBB,
                                           MachineBasicBlock::iterator MI,
                                           unsigned DestReg, int FrameIdx,
                                           const TargetRegisterClass *RC,
                                           const TargetRegisterInfo *TRI) const{
  DebugLoc DL;
  if (MI != MBB.end()) DL = MI->getDebugLoc();
  MachineFunction &MF = *MBB.getParent();
  MachineFrameInfo &MFI = *MF.getFrameInfo();

  MachineMemOperand *MMO =
    MF.getMachineMemOperand(MachinePointerInfo::getFixedStack(FrameIdx),
                            MachineMemOperand::MOLoad,
                            MFI.getObjectSize(FrameIdx),
                            MFI.getObjectAlignment(FrameIdx));

  if (RC == &MAPIP::GR16RegClass || RC == &MAPIP::GEXR16RegClass)
    BuildMI(MBB, MI, DL, get(MAPIP::MOV16rm), DestReg)
      .addFrameIndex(FrameIdx).addImm(0).addMemOperand(MMO);
  else
    llvm_unreachable("Cannot store this register to stack slot!");
}

unsigned MAPIPInstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                              int &FrameIndex) const {
  if (MI->getOpcode() == MAPIP::MOV16rm) {
    if (MI->getOperand(1).isFI()) {
      // MOV reg, [SP+idx]
      // operand 0 is dest reg, 1 is frame index, 2 immediate 0
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }
  return 0;
}

unsigned MAPIPInstrInfo::isStoreToStackSlot(const MachineInstr *MI,
                                            int &FrameIndex) const {
  if (MI->getOpcode() == MAPIP::MOV16mr) {
    if (MI->getOperand(0).isFI()) {
      // MOV [SP+idx], reg
      // operand 0 is frame index, 1 is immediate 0, 2 is register
      FrameIndex = MI->getOperand(0).getIndex();
      return MI->getOperand(2).getReg();
    }
  }
  return 0;
}

void MAPIPInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                  MachineBasicBlock::iterator I, DebugLoc DL,
                                  unsigned DestReg, unsigned SrcReg,
                                  bool KillSrc) const {

  // An interesting aspect of MAPIP is that all the registers,
  // including PC, SP and O are valid SET arguments. So, it's
  // legal to say SET PC, O; It just usually does not make sense.
  unsigned Opc = MAPIP::MOV16rr;

  BuildMI(MBB, I, DL, get(Opc), DestReg)
    .addReg(SrcReg, getKillRegState(KillSrc));
}

static bool isBR_CC(unsigned Opcode) {
  switch (Opcode) {
    default: return false;
    case MAPIP::BR_CCrr:
    case MAPIP::BR_CCri:
    case MAPIP::BR_CCir:
    case MAPIP::BR_CCii:
      return true;
  }
}

unsigned MAPIPInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const {
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;

  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;
    if (I->getOpcode() != MAPIP::JMP &&
        !isBR_CC(I->getOpcode()) &&
        I->getOpcode() != MAPIP::Br &&
        I->getOpcode() != MAPIP::Bm)
      break;
    // Remove the branch.
    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }

  return Count;
}

bool MAPIPInstrInfo::
ReverseBranchCondition(SmallVectorImpl<MachineOperand> &Cond) const {
  assert(Cond.size() == 4 && "Invalid BR_CC condition!");

  MAPIPCC::CondCodes CC = static_cast<MAPIPCC::CondCodes>(Cond[1].getImm());

  switch (CC) {
  default: llvm_unreachable("Invalid branch condition!");

  case MAPIPCC::MCOND_GE:
	CC = MAPIPCC::MCOND_LE;
	break;
  case MAPIPCC::MCOND_LE:
	CC = MAPIPCC::MCOND_GE;
	break;
  case MAPIPCC::MCOND_GEU:
	CC = MAPIPCC::MCOND_LEU;
	break;
  case MAPIPCC::MCOND_LEU:
	CC = MAPIPCC::MCOND_GEU;
	break;
  case MAPIPCC::MCOND_EQ:
    CC = MAPIPCC::MCOND_NE;
    break;
  case MAPIPCC::MCOND_NE:
    CC = MAPIPCC::MCOND_EQ;
    break;
  case MAPIPCC::MCOND_LTU:
    CC = MAPIPCC::MCOND_GTU;
    break;
  case MAPIPCC::MCOND_GTU:
    CC = MAPIPCC::MCOND_LTU;
    break;
  case MAPIPCC::MCOND_LT:
    CC = MAPIPCC::MCOND_GT;
    break;
  case MAPIPCC::MCOND_GT:
    CC = MAPIPCC::MCOND_LT;
    break;
  }

  Cond[1].setImm(CC);
  return false;
}

bool MAPIPInstrInfo::isUnpredicatedTerminator(const MachineInstr *MI) const {
  if (!MI->isTerminator()) return false;

  // Conditional branch is a special case.
  if (MI->isBranch() && !MI->isBarrier())
    return true;
  if (!MI->isPredicable())
    return true;
  return !isPredicated(MI);
}

static bool AcceptsAdditionalEqualityCheck(MAPIPCC::CondCodes simpleCC,
                                           MAPIPCC::CondCodes *complexCC) {
  *complexCC = simpleCC;
  switch (simpleCC) {
  default: llvm_unreachable("Invalid comparison code!");
  case MAPIPCC::MCOND_GE:
  case MAPIPCC::MCOND_LE:
  case MAPIPCC::MCOND_EQ:
  case MAPIPCC::MCOND_NE:
  case MAPIPCC::MCOND_LEU:
  case MAPIPCC::MCOND_GEU:
    llvm_unreachable("Not a simple CC, already contains an equality!");
    return false;
  case MAPIPCC::MCOND_GT:
    *complexCC = MAPIPCC::MCOND_GE;
    return true;
  case MAPIPCC::MCOND_LT:
    *complexCC = MAPIPCC::MCOND_LE;
    return true;
  case MAPIPCC::MCOND_LTU:
    *complexCC = MAPIPCC::MCOND_LEU;
    return true;
  case MAPIPCC::MCOND_GTU:
    *complexCC = MAPIPCC::MCOND_GEU;
    return true;
  }
}

bool MAPIPInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                    MachineBasicBlock *&TBB,
                                    MachineBasicBlock *&FBB,
                                    SmallVectorImpl<MachineOperand> &Cond,
                                    bool AllowModify) const {
  // Start from the bottom of the block and work up, examining the
  // terminator instructions.
  MachineBasicBlock::iterator I = MBB.end();
  while (I != MBB.begin()) {
    --I;
    if (I->isDebugValue())
      continue;

    // Working from the bottom, when we see a non-terminator
    // instruction, we're done.
    if (!isUnpredicatedTerminator(I))
      break;

    // A terminator that isn't a branch can't easily be handled
    // by this analysis.
    if (!I->isBranch())
      return true;

    // Cannot handle indirect branches.
    if (I->getOpcode() == MAPIP::Br ||
        I->getOpcode() == MAPIP::Bm)
      return true;

    // Handle unconditional branches.
    if (I->getOpcode() == MAPIP::JMP) {
      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      // If the block has any instructions after a JMP, delete them.
      while (llvm::next(I) != MBB.end())
        llvm::next(I)->eraseFromParent();
      Cond.clear();
      FBB = 0;

      // Delete the JMP if it's equivalent to a fall-through.
      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
        TBB = 0;
        I->eraseFromParent();
        I = MBB.end();
        continue;
      }

      // TBB is used to indicate the unconditinal destination.
      TBB = I->getOperand(0).getMBB();
      continue;
    }

    // Handle conditional branches.
    assert(isBR_CC(I->getOpcode()) && "Invalid conditional branch");
    MAPIPCC::CondCodes BranchCode =
      static_cast<MAPIPCC::CondCodes>(I->getOperand(0).getImm());
    if (BranchCode == MAPIPCC::COND_INVALID)
      return true;  // Can't handle weird stuff.

    MachineOperand LHS = I->getOperand(1);
    MachineOperand RHS = I->getOperand(2);

    // Working from the bottom, handle the first conditional branch.
    if (Cond.empty()) {
      FBB = TBB;
      TBB = I->getOperand(3).getMBB();
      Cond.push_back(MachineOperand::CreateImm(I->getOpcode()));
      Cond.push_back(MachineOperand::CreateImm(BranchCode));
      Cond.push_back(LHS);
      Cond.push_back(RHS);
      continue;
    }

    assert(Cond.size() == 4);
    assert(TBB);

    // Is it a complex CC?
    MAPIPCC::CondCodes complexCC;
    if ((BranchCode == MAPIPCC::MCOND_EQ)
        && AcceptsAdditionalEqualityCheck((MAPIPCC::CondCodes) Cond[1].getImm(), &complexCC)
        && (TBB == I->getOperand(3).getMBB())
        // This should actually check for equality but that's just too much code...
        && (((Cond[2].getType() == LHS.getType()) && (Cond[3].getType() == RHS.getType()))
          || ((Cond[2].getType() == RHS.getType()) && (Cond[3].getType() == LHS.getType())))) {

      Cond[1] = MachineOperand::CreateImm(complexCC);
    }
  }

  return false;
}

static bool IsComplexCC(MAPIPCC::CondCodes cc,
                        MAPIPCC::CondCodes *simpleCC) {
  *simpleCC = cc;
  switch (cc) {
  default: llvm_unreachable("Invalid condition code!");
/*
  case MAPIPCC::COND_B:
  case MAPIPCC::COND_C:
  case MAPIPCC::COND_E:
  case MAPIPCC::COND_NE:
  case MAPIPCC::COND_G:
  case MAPIPCC::COND_L:
  case MAPIPCC::COND_A:
  case MAPIPCC::COND_U:
    return false;
  case MAPIPCC::COND_GE:
    *simpleCC = MAPIPCC::COND_G;
    return true;
  case MAPIPCC::COND_LE:
    *simpleCC = MAPIPCC::COND_L;
    return true;
  case MAPIPCC::COND_AE:
    *simpleCC = MAPIPCC::COND_A;
    return true;
  case MAPIPCC::COND_UE:
    *simpleCC = MAPIPCC::COND_U;
    return true;
*/
  case MAPIPCC::MCOND_GE:
  case MAPIPCC::MCOND_LE:
  case MAPIPCC::MCOND_GEU:
  case MAPIPCC::MCOND_LEU:
  case MAPIPCC::MCOND_EQ:
  case MAPIPCC::MCOND_NE:
  case MAPIPCC::MCOND_LTU:
  case MAPIPCC::MCOND_GTU:
  case MAPIPCC::MCOND_LT:
  case MAPIPCC::MCOND_GT:
	return false;
  }
}

unsigned
MAPIPInstrInfo::InsertBranch(MachineBasicBlock &MBB, MachineBasicBlock *TBB,
                              MachineBasicBlock *FBB,
                              const SmallVectorImpl<MachineOperand> &Cond,
                              DebugLoc DL) const {
  // Shouldn't be a fall through.
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 4 || Cond.size() == 0) &&
         "MAPIP branch conditions have four components!");

  if (Cond.empty()) {
    // Unconditional branch?
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(MAPIP::JMP)).addMBB(TBB);
    return 1;
  }

  // Conditional branch.
  unsigned Count = 0;
  unsigned Opcode = Cond[0].getImm();
  MAPIPCC::CondCodes CC = (MAPIPCC::CondCodes) Cond[1].getImm();
  MachineOperand LHS = Cond[2];
  MachineOperand RHS = Cond[3];

  // Is it a complex CC?
  MAPIPCC::CondCodes simpleCC;
  if (IsComplexCC(CC, &simpleCC)) {
    BuildMI(&MBB, DL, get(Opcode))
      .addImm(MAPIPCC::MCOND_EQ)
      .addOperand(LHS).addOperand(RHS)
      .addMBB(TBB);
    CC = simpleCC;
    ++Count;
  }
  BuildMI(&MBB, DL, get(Opcode))
    .addImm(CC)
    .addOperand(LHS).addOperand(RHS)
    .addMBB(TBB);
  ++Count;

  if (FBB) {
    // Two-way Conditional branch. Insert the second branch.
    BuildMI(&MBB, DL, get(MAPIP::JMP)).addMBB(FBB);
    ++Count;
  }
  return Count;
}

/// GetInstSize - Return the number of bytes of code the specified
/// instruction may be.  This returns the maximum number of bytes.
///
unsigned MAPIPInstrInfo::GetInstSizeInBytes(const MachineInstr *MI) const {
  const MCInstrDesc &Desc = MI->getDesc();

  switch (Desc.TSFlags & MAPIPII::SizeMask) {
  default:
    switch (Desc.getOpcode()) {
    default: llvm_unreachable("Unknown instruction size!");
    case TargetOpcode::PROLOG_LABEL:
    case TargetOpcode::EH_LABEL:
    case TargetOpcode::IMPLICIT_DEF:
    case TargetOpcode::KILL:
    case TargetOpcode::DBG_VALUE:
      return 0;
    case TargetOpcode::INLINEASM: {
      const MachineFunction *MF = MI->getParent()->getParent();
      const TargetInstrInfo &TII = *MF->getTarget().getInstrInfo();
      return TII.getInlineAsmLength(MI->getOperand(0).getSymbolName(),
                                    *MF->getTarget().getMCAsmInfo());
    }
    }
  case MAPIPII::Size2Bytes:
    return 2;
  case MAPIPII::Size4Bytes:
    return 4;
  case MAPIPII::Size6Bytes:
    return 6;
  }
}
