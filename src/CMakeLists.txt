cmake_minimum_required(VERSION 3.17)
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

target_precompile_headers(ASTConsumer
  PRIVATE
    <algorithm>
    <memory>
    <string>
    <utility>
    <vector>
    ${LLVM_INCLUDE_DIRS}/llvm/Support/Host.h
    ${CLANG_INCLUDE_DIRS}/clang/AST/Decl.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/Builtins.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/Diagnostic.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/FileManager.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/LangOptions.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/SourceManager.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/TargetInfo.h
    ${CLANG_INCLUDE_DIRS}/clang/Basic/TargetOptions.h
    ${CLANG_INCLUDE_DIRS}/clang/Frontend/CompilerInstance.h
    ${CLANG_INCLUDE_DIRS}/clang/Frontend/FrontendOptions.h
    ${CLANG_INCLUDE_DIRS}/clang/Frontend/Utils.h
    ${CLANG_INCLUDE_DIRS}/clang/Lex/HeaderSearch.h
    ${CLANG_INCLUDE_DIRS}/clang/Lex/Preprocessor.h
    ${CLANG_INCLUDE_DIRS}/clang/Parse/ParseAST.h
  )

add_library(cpp_slexer
  STATIC
    instantiate_cpp_exprgrammar.cpp
    instantiate_cpp_grammar.cpp
    instantiate_cpp_literalgrs.cpp
    instantiate_defined_grammar.cpp
    instantiate_slex_lexer.cpp
  )

target_link_libraries(cpp_slexer
  PRIVATE
    Boost::system
)

target_include_directories(cpp_slexer PUBLIC
  ${CMAKE_SOURCE_DIR}/include
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

add_dependencies(cpp2cxx-core ASTConsumer cpp_slexer)

target_link_libraries(cpp2cxx-core
  PRIVATE
    ASTConsumer
    Boost::program_options
    Boost::system
    Boost::wave
    cpp_slexer
    fmt::fmt
  )

target_include_directories(cpp2cxx-core
  PUBLIC
    ${CMAKE_SOURCE_DIR}/include
  PRIVATE
    ${ROBIN_MAP_INCLUDE_DIRS}
  )

target_precompile_headers(cpp2cxx-core
  PRIVATE
    <algorithm>
    <fstream>
    <map>
    <memory>
    <ostream>
    <sstream>
    <string>
    <utility>
    <vector>
    <fmt/core.h>
    <fmt/format.h>
    <fmt/ostream.h>
    <boost/graph/adjacency_list.hpp>
    <boost/graph/depth_first_search.hpp>
    <boost/graph/graph_traits.hpp>
    <boost/graph/topological_sort.hpp>
    <boost/wave.hpp>
  )