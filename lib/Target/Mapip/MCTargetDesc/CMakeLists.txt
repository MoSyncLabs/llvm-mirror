add_llvm_library(LLVMMAPIPDesc
  MAPIPMCTargetDesc.cpp
  MAPIPMCAsmInfo.cpp
  )

add_dependencies(LLVMMAPIPDesc MAPIPCommonTableGen)
