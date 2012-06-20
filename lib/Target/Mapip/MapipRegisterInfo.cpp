//===-- MAPIPRegisterInfo.cpp - MAPIP Register Information --------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
//===----------------------------------------------------------------------===//
//
// This file contains the MAPIP implementation of the TargetRegisterInfo class.
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "mapip-reg-info"

#include "MapipRegisterInfo.h"
#include "Mapip.h"
#include "MapipMachineFunctionInfo.h"
#include "MapipTargetMachine.h"
#include "llvm/Function.h"
#include "llvm/CodeGen/MachineFrameInfo.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetOptions.h"
#include "llvm/ADT/BitVector.h"
#include "llvm/Support/ErrorHandling.h"

#define GET_REGINFO_TARGET_DESC
#include "MapipGenRegisterInfo.inc"

using namespace llvm;

// FIXME: Provide proper call frame setup / destroy opcodes.
MAPIPRegisterInfo::MAPIPRegisterInfo(MAPIPTargetMachine &tm,
                                       const TargetInstrInfo &tii)
  : MAPIPGenRegisterInfo(MAPIP::A), TM(tm), TII(tii) {
  StackAlign = TM.getFrameLowering()->getStackAlignment();
}

const uint16_t*
MAPIPRegisterInfo::getCalleeSavedRegs(const MachineFunction *MF) const {
  const Function* F = MF->getFunction();
  static const uint16_t CalleeSavedRegs[] = {
    MAPIP::X, MAPIP::Y, MAPIP::Z, MAPIP::I, MAPIP::J,
    0
  };
  // In interrupt handlers, the caller has to save all registers except A.
  // This includes EX.
  static const uint16_t CalleeSavedRegsIntr[] = {
 //   MAPIP::EX,
    MAPIP::B, MAPIP::C,
    MAPIP::X, MAPIP::Y, MAPIP::Z, MAPIP::I, MAPIP::J,
    0
  };

  return (/*F->getCallingConv() == CallingConv::MAPIP_INTR ?
          CalleeSavedRegsIntr :*/ CalleeSavedRegs);
}

const uint32_t*
MAPIPRegisterInfo::getCallPreservedMask(CallingConv::ID CallConv) const {
    switch(CallConv) {
        default:
            llvm_unreachable("Unsupported calling convention");
        case CallingConv::C:
        case CallingConv::Fast:
            return CSR_MAPIP_RegMask;
    }
}

BitVector MAPIPRegisterInfo::getReservedRegs(const MachineFunction &MF) const {
  BitVector Reserved(getNumRegs());
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  // Mark 2 special registers as reserved.
//  Reserved.set(MAPIP::EX);
  Reserved.set(MAPIP::SP);

  // Mark frame pointer as reserved if needed.
  if (TFI->hasFP(MF))
//    Reserved.set(MAPIP::J);
    Reserved.set(MAPIP::FR);

  return Reserved;
}

const TargetRegisterClass *
MAPIPRegisterInfo::getPointerRegClass(unsigned Kind) const {
  return &MAPIP::GR16RegClass;
}

void MAPIPRegisterInfo::
eliminateCallFramePseudoInstr(MachineFunction &MF, MachineBasicBlock &MBB,
                              MachineBasicBlock::iterator I) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  if (!TFI->hasReservedCallFrame(MF)) {
    // If the stack pointer can be changed after prologue, turn the
    // adjcallstackup instruction into a 'sub SP, <amt>' and the
    // adjcallstackdown instruction into 'add SP, <amt>'
    // TODO: consider using push / pop instead of sub + store / add
    MachineInstr *Old = I;
    uint64_t Amount = Old->getOperand(0).getImm();
    if (Amount != 0) {
      // We need to keep the stack aligned properly.  To do this, we round the
      // amount of space needed for the outgoing arguments up to the next
      // alignment boundary.
      Amount = (Amount+StackAlign-1)/StackAlign*StackAlign;

      MachineInstr *New = 0;
      if (Old->getOpcode() == TII.getCallFrameSetupOpcode()) {
        New = BuildMI(MF, Old->getDebugLoc(),
                      TII.get(MAPIP::SUB16ri), MAPIP::SP)
          .addReg(MAPIP::SP).addImm(Amount);
      } else {
        assert(Old->getOpcode() == TII.getCallFrameDestroyOpcode());
        // factor out the amount the callee already popped.
        uint64_t CalleeAmt = Old->getOperand(1).getImm();
        Amount -= CalleeAmt;
        if (Amount)
          New = BuildMI(MF, Old->getDebugLoc(),
                        TII.get(MAPIP::ADD16ri), MAPIP::SP)
            .addReg(MAPIP::SP).addImm(Amount);
      }

      if (New) {
        // The SRW implicit def is dead.
        //New->getOperand(3).setIsDead();

        // Replace the pseudo instruction with a new instruction...
        MBB.insert(I, New);
      }
    }
  } else if (I->getOpcode() == TII.getCallFrameDestroyOpcode()) {
    // If we are performing frame pointer elimination and if the callee pops
    // something off the stack pointer, add it back.
    if (uint64_t CalleeAmt = I->getOperand(1).getImm()) {
      MachineInstr *Old = I;
      MachineInstr *New =
        BuildMI(MF, Old->getDebugLoc(), TII.get(MAPIP::SUB16ri),
                MAPIP::SP).addReg(MAPIP::SP).addImm(CalleeAmt);
      // The SRW implicit def is dead.
      //New->getOperand(3).setIsDead();

      MBB.insert(I, New);
    }
  }

  MBB.erase(I);
}

void
MAPIPRegisterInfo::eliminateFrameIndex(MachineBasicBlock::iterator II,
                                        int SPAdj, RegScavenger *RS) const {
  assert(SPAdj == 0 && "Unexpected");

  unsigned i = 0;
  MachineInstr &MI = *II;
  MachineBasicBlock &MBB = *MI.getParent();
  MachineFunction &MF = *MBB.getParent();
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();
  DebugLoc dl = MI.getDebugLoc();
  while (!MI.getOperand(i).isFI()) {
    ++i;
    assert(i < MI.getNumOperands() && "Instr doesn't have FrameIndex operand!");
  }

  int FrameIndex = MI.getOperand(i).getIndex();

  unsigned BasePtr = (TFI->hasFP(MF) ? MAPIP::FR/*MAPIP::J*/ : MAPIP::SP);
  int Offset = MF.getFrameInfo()->getObjectOffset(FrameIndex);

  // Skip the saved PC
  //Offset += 1;

  if (!TFI->hasFP(MF))
    Offset += MF.getFrameInfo()->getStackSize();

  // Fold imm into offset
  Offset += MI.getOperand(i+1).getImm();

  if (MI.getOpcode() == MAPIP::ADD16ri) {
    // This is actually "load effective address" of the stack slot
    // instruction. We have only two-address instructions, thus we need to
    // expand it into mov + add

    MI.setDesc(TII.get(MAPIP::MOV16rr));
    MI.getOperand(i).ChangeToRegister(BasePtr, false);

    if (Offset == 0)
      return;

    // We need to materialize the offset via add instruction.
    unsigned DstReg = MI.getOperand(0).getReg();
    if (Offset < 0)
      BuildMI(MBB, llvm::next(II), dl, TII.get(MAPIP::SUB16ri), DstReg)
        .addReg(DstReg).addImm(-Offset);
    else
      BuildMI(MBB, llvm::next(II), dl, TII.get(MAPIP::ADD16ri), DstReg)
        .addReg(DstReg).addImm(Offset);

    return;
  }

  MI.getOperand(i).ChangeToRegister(BasePtr, false);
  MI.getOperand(i+1).ChangeToImmediate(Offset);
}

unsigned MAPIPRegisterInfo::getFrameRegister(const MachineFunction &MF) const {
  const TargetFrameLowering *TFI = MF.getTarget().getFrameLowering();

  return TFI->hasFP(MF) ? /*MAPIP::J*/MAPIP::FR : MAPIP::SP;
}
