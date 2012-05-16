add_llvm_library(LLVMMapipDesc
  MapipMCTargetDesc.cpp
  MapipMCAsmInfo.cpp
  )

add_dependencies(LLVMMapipDesc MapipCommonTableGen)
