//===-- MapipInstrInfo.cpp - Mapip Instruction Information ----------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the Mapip implementation of the TargetInstrInfo class.
//
//===----------------------------------------------------------------------===//

#include "MapipInstrInfo.h"
#include "Mapip.h"
#include "MapipMachineFunctionInfo.h"
#include "MapipSubtarget.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/ErrorHandling.h"
#include "llvm/Support/TargetRegistry.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/ADT/SmallVector.h"

#define GET_INSTRINFO_CTOR
#include "MapipGenInstrInfo.inc"

using namespace llvm;

MapipInstrInfo::MapipInstrInfo(MapipSubtarget &ST)
  : MapipGenInstrInfo(MAPIP::ADJCALLSTACKDOWN, MAPIP::ADJCALLSTACKUP),
    RI(ST, *this), Subtarget(ST) {
}

/// isLoadFromStackSlot - If the specified machine instruction is a direct
/// load from a stack slot, return the virtual or physical register number of
/// the destination along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than loading from the stack slot.
unsigned MapipInstrInfo::isLoadFromStackSlot(const MachineInstr *MI,
                                             int &FrameIndex) const {
  if (MI->getOpcode() == MAPIP::LDri ||
      MI->getOpcode() == MAPIP::LDFri ||
      MI->getOpcode() == MAPIP::LDDFri) {
    if (MI->getOperand(1).isFI() && MI->getOperand(2).isImm() &&
        MI->getOperand(2).getImm() == 0) {
      FrameIndex = MI->getOperand(1).getIndex();
      return MI->getOperand(0).getReg();
    }
  }
  return 0;
}

/// isStoreToStackSlot - If the specified machine instruction is a direct
/// store to a stack slot, return the virtual or physical register number of
/// the source reg along with the FrameIndex of the loaded stack slot.  If
/// not, return 0.  This predicate must return 0 if the instruction has
/// any side effects other than storing to the stack slot.
unsigned MapipInstrInfo::isStoreToStackSlot(const MachineInstr *MI,
                                            int &FrameIndex) const {
  if (MI->getOpcode() == MAPIP::STri ||
      MI->getOpcode() == MAPIP::STFri ||
      MI->getOpcode() == MAPIP::STDFri) {
    if (MI->getOperand(0).isFI() && MI->getOperand(1).isImm() &&
        MI->getOperand(1).getImm() == 0) {
      FrameIndex = MI->getOperand(0).getIndex();
      return MI->getOperand(2).getReg();
    }
  }
  return 0;
}

static bool IsIntegerCC(unsigned CC)
{
  return  (CC <= SPCC::ICC_VC);
}


static SPCC::CondCodes GetOppositeBranchCondition(SPCC::CondCodes CC)
{
  switch(CC) {
  case SPCC::ICC_NE:   return SPCC::ICC_E;
  case SPCC::ICC_E:    return SPCC::ICC_NE;
  case SPCC::ICC_G:    return SPCC::ICC_LE;
  case SPCC::ICC_LE:   return SPCC::ICC_G;
  case SPCC::ICC_GE:   return SPCC::ICC_L;
  case SPCC::ICC_L:    return SPCC::ICC_GE;
  case SPCC::ICC_GU:   return SPCC::ICC_LEU;
  case SPCC::ICC_LEU:  return SPCC::ICC_GU;
  case SPCC::ICC_CC:   return SPCC::ICC_CS;
  case SPCC::ICC_CS:   return SPCC::ICC_CC;
  case SPCC::ICC_POS:  return SPCC::ICC_NEG;
  case SPCC::ICC_NEG:  return SPCC::ICC_POS;
  case SPCC::ICC_VC:   return SPCC::ICC_VS;
  case SPCC::ICC_VS:   return SPCC::ICC_VC;

  case SPCC::FCC_U:    return SPCC::FCC_O;
  case SPCC::FCC_O:    return SPCC::FCC_U;
  case SPCC::FCC_G:    return SPCC::FCC_LE;
  case SPCC::FCC_LE:   return SPCC::FCC_G;
  case SPCC::FCC_UG:   return SPCC::FCC_ULE;
  case SPCC::FCC_ULE:  return SPCC::FCC_UG;
  case SPCC::FCC_L:    return SPCC::FCC_GE;
  case SPCC::FCC_GE:   return SPCC::FCC_L;
  case SPCC::FCC_UL:   return SPCC::FCC_UGE;
  case SPCC::FCC_UGE:  return SPCC::FCC_UL;
  case SPCC::FCC_LG:   return SPCC::FCC_UE;
  case SPCC::FCC_UE:   return SPCC::FCC_LG;
  case SPCC::FCC_NE:   return SPCC::FCC_E;
  case SPCC::FCC_E:    return SPCC::FCC_NE;
  }
  llvm_unreachable("Invalid cond code");
}

MachineInstr *
MapipInstrInfo::emitFrameIndexDebugValue(MachineFunction &MF,
                                         int FrameIx,
                                         uint64_t Offset,
                                         const MDNode *MDPtr,
                                         DebugLoc dl) const {
  MachineInstrBuilder MIB = BuildMI(MF, dl, get(MAPIP::DBG_VALUE))
    .addFrameIndex(FrameIx).addImm(0).addImm(Offset).addMetadata(MDPtr);
  return &*MIB;
}


bool MapipInstrInfo::AnalyzeBranch(MachineBasicBlock &MBB,
                                   MachineBasicBlock *&TBB,
                                   MachineBasicBlock *&FBB,
                                   SmallVectorImpl<MachineOperand> &Cond,
                                   bool AllowModify) const
{

  MachineBasicBlock::iterator I = MBB.end();
  MachineBasicBlock::iterator UnCondBrIter = MBB.end();
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugValue())
      continue;

    //When we see a non-terminator, we are done
    if (!isUnpredicatedTerminator(I))
      break;

    //Terminator is not a branch
    if (!I->isBranch())
      return true;

    //Handle Unconditional branches
    if (I->getOpcode() == MAPIP::BA) {
      UnCondBrIter = I;

      if (!AllowModify) {
        TBB = I->getOperand(0).getMBB();
        continue;
      }

      while (llvm::next(I) != MBB.end())
        llvm::next(I)->eraseFromParent();

      Cond.clear();
      FBB = 0;

      if (MBB.isLayoutSuccessor(I->getOperand(0).getMBB())) {
        TBB = 0;
        I->eraseFromParent();
        I = MBB.end();
        UnCondBrIter = MBB.end();
        continue;
      }

      TBB = I->getOperand(0).getMBB();
      continue;
    }

    unsigned Opcode = I->getOpcode();
    if (Opcode != MAPIP::BCOND && Opcode != MAPIP::FBCOND)
      return true; //Unknown Opcode

    SPCC::CondCodes BranchCode = (SPCC::CondCodes)I->getOperand(1).getImm();

    if (Cond.empty()) {
      MachineBasicBlock *TargetBB = I->getOperand(0).getMBB();
      if (AllowModify && UnCondBrIter != MBB.end() &&
          MBB.isLayoutSuccessor(TargetBB)) {

        //Transform the code
        //
        //    brCC L1
        //    ba L2
        // L1:
        //    ..
        // L2:
        //
        // into
        //
        //   brnCC L2
        // L1:
        //   ...
        // L2:
        //
        BranchCode = GetOppositeBranchCondition(BranchCode);
        MachineBasicBlock::iterator OldInst = I;
        BuildMI(MBB, UnCondBrIter, MBB.findDebugLoc(I), get(Opcode))
          .addMBB(UnCondBrIter->getOperand(0).getMBB()).addImm(BranchCode);
        BuildMI(MBB, UnCondBrIter, MBB.findDebugLoc(I), get(MAPIP::BA))
          .addMBB(TargetBB);

        OldInst->eraseFromParent();
        UnCondBrIter->eraseFromParent();

        UnCondBrIter = MBB.end();
        I = MBB.end();
        continue;
      }
      FBB = TBB;
      TBB = I->getOperand(0).getMBB();
      Cond.push_back(MachineOperand::CreateImm(BranchCode));
      continue;
    }
    //FIXME: Handle subsequent conditional branches
    //For now, we can't handle multiple conditional branches
    return true;
  }
  return false;
}

unsigned
MapipInstrInfo::InsertBranch(MachineBasicBlock &MBB,MachineBasicBlock *TBB,
                             MachineBasicBlock *FBB,
                             const SmallVectorImpl<MachineOperand> &Cond,
                             DebugLoc DL) const {
  assert(TBB && "InsertBranch must not be told to insert a fallthrough");
  assert((Cond.size() == 1 || Cond.size() == 0) &&
         "Mapip branch conditions should have one component!");

  if (Cond.empty()) {
    assert(!FBB && "Unconditional branch with multiple successors!");
    BuildMI(&MBB, DL, get(MAPIP::BA)).addMBB(TBB);
    return 1;
  }

  //Conditional branch
  unsigned CC = Cond[0].getImm();

  if (IsIntegerCC(CC))
    BuildMI(&MBB, DL, get(MAPIP::BCOND)).addMBB(TBB).addImm(CC);
  else
    BuildMI(&MBB, DL, get(MAPIP::FBCOND)).addMBB(TBB).addImm(CC);
  if (!FBB)
    return 1;

  BuildMI(&MBB, DL, get(MAPIP::BA)).addMBB(FBB);
  return 2;
}

unsigned MapipInstrInfo::RemoveBranch(MachineBasicBlock &MBB) const
{
  MachineBasicBlock::iterator I = MBB.end();
  unsigned Count = 0;
  while (I != MBB.begin()) {
    --I;

    if (I->isDebugValue())
      continue;

    if (I->getOpcode() != MAPIP::BA
        && I->getOpcode() != MAPIP::BCOND
        && I->getOpcode() != MAPIP::FBCOND)
      break; // Not a branch

    I->eraseFromParent();
    I = MBB.end();
    ++Count;
  }
  return Count;
}

void MapipInstrInfo::copyPhysReg(MachineBasicBlock &MBB,
                                 MachineBasicBlock::iterator I, DebugLoc DL,
                                 unsigned DestReg, unsigned SrcReg,
                                 bool KillSrc) const {
  if (MAPIP::IntRegsRegClass.contains(DestReg, SrcReg))
    BuildMI(MBB, I, DL, get(MAPIP::ORrr), DestReg).addReg(MAPIP::G0)
      .addReg(SrcReg, getKillRegState(KillSrc));
  else if (MAPIP::FPRegsRegClass.contains(DestReg, SrcReg))
    BuildMI(MBB, I, DL, get(MAPIP::FMOVS), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  else if (MAPIP::DFPRegsRegClass.contains(DestReg, SrcReg))
    BuildMI(MBB, I, DL, get(Subtarget.isV9() ? MAPIP::FMOVD : MAPIP::FpMOVD), DestReg)
      .addReg(SrcReg, getKillRegState(KillSrc));
  else
    llvm_unreachable("Impossible reg-to-reg copy");
}

void MapipInstrInfo::
storeRegToStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                    unsigned SrcReg, bool isKill, int FI,
                    const TargetRegisterClass *RC,
                    const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  // On the order of operands here: think "[FrameIdx + 0] = SrcReg".
  if (RC == &MAPIP::IntRegsRegClass)
    BuildMI(MBB, I, DL, get(MAPIP::STri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg, getKillRegState(isKill));
  else if (RC == &MAPIP::FPRegsRegClass)
    BuildMI(MBB, I, DL, get(MAPIP::STFri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg,  getKillRegState(isKill));
  else if (RC == &MAPIP::DFPRegsRegClass)
    BuildMI(MBB, I, DL, get(MAPIP::STDFri)).addFrameIndex(FI).addImm(0)
      .addReg(SrcReg,  getKillRegState(isKill));
  else
    llvm_unreachable("Can't store this register to stack slot");
}

void MapipInstrInfo::
loadRegFromStackSlot(MachineBasicBlock &MBB, MachineBasicBlock::iterator I,
                     unsigned DestReg, int FI,
                     const TargetRegisterClass *RC,
                     const TargetRegisterInfo *TRI) const {
  DebugLoc DL;
  if (I != MBB.end()) DL = I->getDebugLoc();

  if (RC == &MAPIP::IntRegsRegClass)
    BuildMI(MBB, I, DL, get(MAPIP::LDri), DestReg).addFrameIndex(FI).addImm(0);
  else if (RC == &MAPIP::FPRegsRegClass)
    BuildMI(MBB, I, DL, get(MAPIP::LDFri), DestReg).addFrameIndex(FI).addImm(0);
  else if (RC == &MAPIP::DFPRegsRegClass)
    BuildMI(MBB, I, DL, get(MAPIP::LDDFri), DestReg).addFrameIndex(FI).addImm(0);
  else
    llvm_unreachable("Can't load this register from stack slot");
}

unsigned MapipInstrInfo::getGlobalBaseReg(MachineFunction *MF) const
{
  MapipMachineFunctionInfo *MapipFI = MF->getInfo<MapipMachineFunctionInfo>();
  unsigned GlobalBaseReg = MapipFI->getGlobalBaseReg();
  if (GlobalBaseReg != 0)
    return GlobalBaseReg;

  // Insert the set of GlobalBaseReg into the first MBB of the function
  MachineBasicBlock &FirstMBB = MF->front();
  MachineBasicBlock::iterator MBBI = FirstMBB.begin();
  MachineRegisterInfo &RegInfo = MF->getRegInfo();

  GlobalBaseReg = RegInfo.createVirtualRegister(&MAPIP::IntRegsRegClass);


  DebugLoc dl;

  BuildMI(FirstMBB, MBBI, dl, get(MAPIP::GETPCX), GlobalBaseReg);
  MapipFI->setGlobalBaseReg(GlobalBaseReg);
  return GlobalBaseReg;
}
