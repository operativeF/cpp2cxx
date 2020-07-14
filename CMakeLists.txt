cmake_minimum_required(VERSION 3.10)

if(${CMAKE_VERSION} VERSION_LESS 3.10)
  cmake_policy(VERSION ${CMAKE_MAJOR_VERSION}.${CMAKE_MINOR_VERSION})
endif()

include("C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")

project(demacrofier
  DESCRIPTION "An automated system for identifying and potentially replacing macros with modern C++ constructs."
  VERSION 2.0
  LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)

find_package(LLVM REQUIRED CONFIG)

include_directories(${LLVM_INCLUDE_DIRS})
add_definitions(${LLVM_DEFINITIONS})

find_package(Clang REQUIRED CONFIG)

include_directories(${CLANG_INCLUDE_DIRS})
add_definitions(${CLANG_DEFINITIONS})

find_package(Boost 1.55 REQUIRED COMPONENTS program_options system wave)

include_directories(${Boost_INCLUDE_DIRS})

set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR}/lib)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR}/bin)
set(HEADER_OUTPUT_PATH ${CMAKE_BINARY_DIR}/include)

include_directories(${CMAKE_BINARY_DIR}/lib)
include_directories(${CMAKE_BINARY_DIR}/bin)
include_directories(${CMAKE_BINARY_DIR}/include)

###############################################################################
#THIS HAS TO BE ADDED BEFORE SUPPLYING THE COMPILER FLAGS FOR THIS DIRECTORY FILES
#THE CLANG LIBRARIES COMPILE WITH THE -fno-rtti FLAGS
#AFTER THE LIBRARY HAS BEEN BUILT WE NEED TO SET THE COMPILER FLAGS ONCE AGAIN 
#BECAUSE THE BOOST LIBRARIES USE rtti
add_subdirectory(ClangInterface)

###############################################################################
# Compiler options
if(CMAKE_COMPILER_IS_GNUCXX)
  set(LLVM_CXXFLAGS "-fPIC -fvisibility-inlines-hidden")
  target_compile_features(cpp2cxx PRIVATE "-pedantic -Wall -std=c++17 -g ${LLVM_CXXFLAGS}")
  message(STATUS "GCC detected - Adding compiler flags ${CMAKE_CXX_FLAGS}")
elseif(MSVC)
  message(STATUS "MSVC detected - Adding compiler flags")
endif(CMAKE_COMPILER_IS_GNUCXX)

###############################################################################
#INCLUDE OTHER SUBDIRECTORIES HERE WHICH ARE FINE WITH -fno-rtti.
add_subdirectory(general_utilities)

###############################################################################
include_directories("${CMAKE_SOURCE_DIR}/general_utilities")

############## INCLUDE DIRECTORIES RELATED TO CLANG ###########################
include_directories("${CMAKE_SOURCE_DIR}")
include_directories("${CMAKE_SOURCE_DIR}/ClangInterface")

############################ DEBUG MACROS #####################################
#add_definitions(-DBUILD_NEW_MACRO_LIST)
add_definitions(-DCLANG_AST_DEBUG_PRINT)
#add_definitions(-DDEBUG_PARSER_PRINT)
#add_definitions(-DDEBUG_CONDITIONALS_PRINT)
#add_definitions(-DDEBUG_CONDITIONALS1_PRINT)
#add_definitions(-DDEBUG_DEMACROFIER_PRINT)
#add_definitions(-DDEBUG_RLPARSER_PRINT)
#add_definitions(-DDEBUG_RLPARSER2_PRINT)
#add_definitions(-DDEBUG_TREE_PRINT)
#add_definitions(-DBUILD_NEW_MACRO_LIST_PRINT)
#add_definitions(-DDEBUG_MACRO_DEPENDENCY_PRINT)
#add_definitions(-DDEBUG_MACRO_DEPENDENCY2_PRINT)
#add_definitions(-DENABLE_WARNING_PRINT)
#add_definitions(-DDEBUG_MACRO_CLASS_PRINT)
#add_definitions(-DDEBUG_MACRO_CLASS2_PRINT)
#add_definitions(-DDEBUG_MACRO_USE_CASE_PRINT)
#add_definitions(-DDEBUG_CLEANUP_CODE_PRINT)
#add_definitions(-DDEBUG_VALIDATOR_PRINT)
add_definitions(-DDEBUG_SUGGESTION_PRINT)
#add_definitions(-DGMACROS="${HEADER_OUTPUT_PATH}/gMacros.dat")

############## DEFINITIONS RELATED TO CLANG ###################################
add_definitions(-D__STDC_CONSTANT_MACROS)
add_definitions(-D__STDC_FORMAT_MACROS)
add_definitions(-D__STDC_LIMIT_MACROS)
add_definitions(-D_GNU_SOURCE)

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
  clangFrontendTool
  clangFrontend
  clangDriver
  clangSerialization
  clangCodeGen
  clangParse
  clangSema
  clangAnalysis
  clangEdit
  clangAST
  clangLex
  clangBasic
)

###############################################################################
add_executable(cpp2cxx-suggest
    MainSuggest.cpp
)

target_link_libraries(cpp2cxx-suggest
    cpp2cxx-core
    Boost::system
    Boost::wave
    Boost::program_options
    ASTConsumer
    cpp2cxx-debug
)

add_executable(cpp2cxx-finalize
    MainFinalize.cpp
)

target_link_libraries(cpp2cxx-finalize
    cpp2cxx-core
    Boost::system
    Boost::wave
    Boost::program_options
    ASTConsumer
    cpp2cxx-debug
)

file(COPY "cpp2cxx-validate.py" DESTINATION ${EXECUTABLE_OUTPUT_PATH})
file(COPY "gMacros.dat" DESTINATION ${HEADER_OUTPUT_PATH})