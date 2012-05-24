set(LLVM_TARGET_DEFINITIONS MAPIP.td)

tablegen(LLVM MAPIPGenRegisterInfo.inc -gen-register-info)
tablegen(LLVM MAPIPGenInstrInfo.inc -gen-instr-info)
tablegen(LLVM MAPIPGenAsmWriter.inc -gen-asm-writer)
tablegen(LLVM MAPIPGenDAGISel.inc -gen-dag-isel)
tablegen(LLVM MAPIPGenCallingConv.inc -gen-callingconv)
tablegen(LLVM MAPIPGenSubtargetInfo.inc -gen-subtarget)
add_public_tablegen_target(MAPIPCommonTableGen)

add_llvm_target(MAPIPCodeGen
  MAPIPISelDAGToDAG.cpp
  MAPIPISelLowering.cpp
  MAPIPInstrInfo.cpp
  MAPIPFrameLowering.cpp
  MAPIPMachineFunctionInfo.cpp
  MAPIPRegisterInfo.cpp
  MAPIPSubtarget.cpp
  MAPIPTargetMachine.cpp
  MAPIPSelectionDAGInfo.cpp
  MAPIPAsmPrinter.cpp
  MAPIPMCInstLower.cpp
  MAPIPPeephole.cpp
  )

add_subdirectory(InstPrinter)
add_subdirectory(TargetInfo)
add_subdirectory(MCTargetDesc)
