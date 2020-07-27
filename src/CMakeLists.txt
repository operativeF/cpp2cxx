cmake_minimum_required(VERSION 3.10)
project(ASTConsumer)

add_library(ASTConsumer
  STATIC
    TrackMacro.cpp
    ASTConsumer.cpp
  )

target_include_directories(ASTConsumer PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  ${LLVM_INCLUDE_DIRS}
  ${CLANG_INCLUDE_DIRS}
  )

target_link_libraries(ASTConsumer
  clangAST
  clangFrontend
  fmt::fmt
  )

add_library(cpp2cxx-core
  STATIC
    RlCategory.cpp
    ReplacementList.cpp
    RlParser.cpp
    Macro.cpp
    MacroStat.cpp
    UseCaseState.cpp
    DepGraph.cpp
    CondParser.cpp
    Demacrofier.cpp
    Parser.cpp
    FileManager.cpp
    Overseer.cpp
    ConfigScheme.cpp
  )

add_dependencies(cpp2cxx-core ASTConsumer)

target_link_libraries(cpp2cxx-core
  ASTConsumer
  Boost::system
  Boost::wave
  Boost::program_options
  fmt::fmt
  )

target_include_directories(cpp2cxx-core PUBLIC
  ${CMAKE_SOURCE_DIR}/include
  )

add_library(cpp2cxx-debug
  STATIC
    debug.cpp
)

target_include_directories(cpp2cxx-debug
  PRIVATE
    ${CMAKE_SOURCE_DIR}/include
)