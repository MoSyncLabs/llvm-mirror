set(LLVM_TARGET_DEFINITIONS Mapip.td)

tablegen(LLVM MapipGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MapipGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MapipGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MapipGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM MapipGenSubtargetInfo.inc -gen-subtarget)
tablegen(LLVM MapipGenCallingConv.inc -gen-callingconv)
add_public_tablegen_target(MapipCommonTableGen)

add_llvm_target(MapipCodeGen
  DelaySlotFiller.cpp
  FPMover.cpp
  MapipAsmPrinter.cpp
  MapipInstrInfo.cpp
  MapipISelDAGToDAG.cpp
  MapipISelLowering.cpp
  MapipFrameLowering.cpp
  MapipMachineFunctionInfo.cpp
  MapipRegisterInfo.cpp
  MapipSubtarget.cpp
  MapipTargetMachine.cpp
  MapipSelectionDAGInfo.cpp
  )

add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
