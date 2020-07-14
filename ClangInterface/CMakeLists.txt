CMAKE_MINIMUM_REQUIRED(VERSION 2.8)
PROJECT(ASTConsumer)

include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/general_utilities")

ADD_DEFINITIONS(-D__STDC_CONSTANT_MACROS)
ADD_DEFINITIONS(-D__STDC_FORMAT_MACROS)
ADD_DEFINITIONS(-D__STDC_LIMIT_MACROS)
ADD_DEFINITIONS(-D_GNU_SOURCE)
ADD_DEFINITIONS(-DCLANG_AST_DEBUG_PRINT)

#SET(CMAKE_CXX_FLAGS ${CMAKE_CXX_FLAGS}  "-std=c++11 -fno-rtti -g")

LINK_DIRECTORIES("${CFE_PREFIX_PATH}/lib")

ADD_LIBRARY(ASTConsumer STATIC
    TrackMacro.cpp
    ASTConsumer.cpp
  )

TARGET_LINK_LIBRARIES(ASTConsumer
  # Output of llvm-config.
LLVMLTO LLVMObjCARCOpts LLVMLinker LLVMBitWriter LLVMIRReader LLVMMIRParser LLVMAsmParser LLVMLibDriver
LLVMOption LLVMDebugInfoPDB LLVMTableGen LLVMOrcJIT LLVMPasses LLVMipo LLVMVectorize LLVMX86Disassembler
LLVMX86AsmParser LLVMX86CodeGen LLVMSelectionDAG LLVMAsmPrinter LLVMX86Desc LLVMMCDisassembler LLVMX86Info
LLVMMCJIT LLVMDebugInfoDWARF LLVMLineEditor LLVMInterpreter
LLVMExecutionEngine LLVMRuntimeDyld LLVMCodeGen LLVMTarget LLVMScalarOpts LLVMProfileData LLVMObject
LLVMMCParser LLVMBitReader LLVMInstCombine LLVMInstrumentation LLVMTransformUtils LLVMMC
LLVMAnalysis LLVMCore LLVMSupport
# Clang frontend tools.
clangFrontendTool clangFrontend clangDriver clangSerialization clangCodeGen clangParse clangSema clangAnalysis clangEdit clangAST clangLex clangBasic
# system libraries.
  #LLVMOption LLVMSelectionDAG LLVMInterpreter LLVMCodeGen LLVMScalarOpts LLVMInstCombine LLVMTransformUtils LLVMipa LLVMAnalysis LLVMTarget LLVMRuntimeDyld LLVMExecutionEngine LLVMMC LLVMObject LLVMBitReader LLVMCore LLVMSupport
  )
