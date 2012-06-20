//===-- MAPIPPeephole.cpp - MAPIP Peephole Optimiztions ---------------===//
//
//                     The LLVM Compiler Infrastructure
//
// This file is distributed under the University of Illinois Open Source
// License. See LICENSE.TXT for details.
//
// This peephole pass optimizes in the following cases.
// 1. Optimises redundant AND+IFE/IFN patterns
//
//===----------------------------------------------------------------------===//

#define DEBUG_TYPE "MAPIP-peephole"
#include "Mapip.h"
#include "MapipTargetMachine.h"
#include "MapipInstrInfo.h"
#include "llvm/Constants.h"
#include "llvm/PassSupport.h"
#include "llvm/ADT/DenseMap.h"
#include "llvm/ADT/Statistic.h"
#include "llvm/CodeGen/Passes.h"
#include "llvm/CodeGen/MachineFunction.h"
#include "llvm/CodeGen/MachineFunctionPass.h"
#include "llvm/CodeGen/MachineInstrBuilder.h"
#include "llvm/CodeGen/MachineRegisterInfo.h"
#include "llvm/Support/CommandLine.h"
#include "llvm/Support/Debug.h"
#include "llvm/Support/raw_ostream.h"
#include "llvm/Target/TargetMachine.h"
#include "llvm/Target/TargetRegisterInfo.h"
#include "llvm/Target/TargetInstrInfo.h"
#include "llvm/MC/MCSymbol.h"
#include <algorithm>
#include <iostream>

using namespace llvm;

static cl::opt<bool> DisableMAPIPPeephole(
  "disable-mapip-peephole",
  cl::Hidden,
  cl::ZeroOrMore,
  cl::init(false),
  cl::desc("Disable Peephole Optimisations")
);

static cl::opt<bool> DisableOptBrcc(
  "disable-mapip-brcc",
  cl::Hidden,
  cl::ZeroOrMore,
  cl::init(false),
  cl::desc("Disable Conditional Branch Optimization")
);

namespace {
  struct MAPIPPeephole : public MachineFunctionPass {
    const MAPIPInstrInfo     *QII;
    const MAPIPRegisterInfo  *QRI;
    const MachineRegisterInfo *MRI;

  public:
    static char ID;
    MAPIPPeephole() : MachineFunctionPass(ID) { }

    bool runOnMachineFunction(MachineFunction &MF);
    void runOptBrcc(MachineBasicBlock *mbb);
    bool swapOptBrcc(MachineInstr *brInstr, MachineInstr *andInstr);

    const char *getPassName() const {
      return "MAPIP optimize conditional branches";
    }

    void getAnalysisUsage(AnalysisUsage &AU) const {
      MachineFunctionPass::getAnalysisUsage(AU);
    }
  };
}

char MAPIPPeephole::ID = 0;

bool MAPIPPeephole::swapOptBrcc(MachineInstr *brInstr, MachineInstr *andInstr) {
  MachineOperand &brA = brInstr->getOperand(1);
  MachineOperand &brB = brInstr->getOperand(2);
  MachineOperand &andA = andInstr->getOperand(1);
  MachineOperand &andB = andInstr->getOperand(2);
  
  if(andB.isReg()) {
    brA.ChangeToRegister(
      andA.getReg(),
      andA.isDef(),
      andA.isImplicit(),
      andA.isKill(),
      andA.isDead(),
      andA.isUndef(),
      andA.isDebug()
    );
    
    brB.ChangeToRegister(
      andB.getReg(),
      andB.isDef(),
      andB.isImplicit(),
      andB.isKill(),
      andB.isDead(),
      andB.isUndef(),
      andB.isDebug()
    );
    
    return true;
  } else if(andB.isImm()) {
    brA.ChangeToRegister(
      andA.getReg(),
      andA.isDef(),
      andA.isImplicit(),
      andA.isKill(),
      andA.isDead(),
      andA.isUndef(),
      andA.isDebug()
    );
    
    brB.ChangeToImmediate(andB.getImm());
    
    return true;
  } else {
    llvm_unreachable("Encountered unexpected combination in swapOptBrcc");
    return false;
  }
}

void MAPIPPeephole::runOptBrcc(MachineBasicBlock *mbb) {  
  DenseMap<unsigned, MachineInstr *> peepholeMap;
  
  for(MachineBasicBlock::iterator miiIter = mbb->begin(); miiIter != mbb->end(); ++miiIter) {
    MachineInstr *instruction = miiIter;  
    
    switch(instruction->getOpcode()) {
      // And instructions
      case MAPIP::AND16ri:
      case MAPIP::AND16rr: {
        assert(instruction->getNumOperands() == 4);
        
        MachineOperand &result = instruction->getOperand(0);
        unsigned resultReg = result.getReg();
        
        if(TargetRegisterInfo::isVirtualRegister(resultReg))
          peepholeMap[resultReg] = instruction;
        
        break;
      }
      
      // Branch instructions
/*
      case MAPIP::BR_CCri: {
        assert(instruction->getNumOperands() == 4);
        
        if(instruction->getOperand(2).getImm() != 0) break; // Only works if comparing to 0
        
        MachineOperand &activeOperand = instruction->getOperand(1);
        unsigned activeReg = activeOperand.getReg();
        
		// Doesn't work if the variable is used after jumping
        if(!activeOperand.isKill() && !activeOperand.isDead()) break;
        
        if(MachineInstr *peepholeSource = peepholeMap.lookup(activeReg)) {              
          // Change the branch instruction

          if(instruction->getOperand(0).getImm() == MAPIPCC::MCOND_NE) {
            instruction->getOperand(0).setImm(MAPIPCC::COND_B);
          } else if(instruction->getOperand(0).getImm() == MAPIPCC::MCOND_EQ) {
            instruction->getOperand(0).setImm(MAPIPCC::COND_C);
          } else { peepholeMap.erase(activeReg); break; }
          
          swapOptBrcc(instruction, peepholeSource);
          
          // Remove the AND from the block
          peepholeSource->eraseFromParent();
        }
      }
*/
    }
  }
}

bool MAPIPPeephole::runOnMachineFunction(MachineFunction &MF) {
  QII = static_cast<const MAPIPInstrInfo *>(MF.getTarget().getInstrInfo());
  QRI = static_cast<const MAPIPRegisterInfo *>(MF.getTarget().getRegisterInfo());
  MRI = &MF.getRegInfo();
  
  // Disable all peephole optimisations
  if(DisableMAPIPPeephole) return false;

  // Loop over all of the basic blocks.
  for(MachineFunction::iterator mbbIter = MF.begin(); mbbIter != MF.end(); ++mbbIter) {
    MachineBasicBlock *mbb = mbbIter;
    
    if(!DisableOptBrcc) runOptBrcc(mbb);
  } // Basic Block
  return true;
}

FunctionPass *llvm::createMAPIPPeephole() {
  return new MAPIPPeephole();
}

