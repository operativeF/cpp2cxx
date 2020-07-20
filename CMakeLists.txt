cmake_minimum_required(VERSION 3.10)

project(demacrofier VERSION 2.0 LANGUAGES CXX)

#set(CMAKE_TOOLCHAIN_FILE $ENV{CMAKE_TOOLCHAIN_FILE})
include("C:/dev/vcpkg/scripts/buildsystems/vcpkg.cmake")

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /std:c++latest")

if(CMAKE_PROJECT_NAME STREQUAL demacrofier)
  set(CMAKE_CXX_EXTENSIONS OFF)

  set_property(GLOBAL PROPERTY USE_FOLDERS ON)

  include(CTest)

endif()

find_package(Boost 1.70.0 REQUIRED COMPONENTS program_options wave system)

find_package(LLVM REQUIRED CONFIG)

find_package(Clang REQUIRED CONFIG)

message(STATUS "Found LLVM ${LLVM_PACKAGE_VERSION}")
message(STATUS "Using LLVMConfig.cmake in: ${LLVM_DIR}")

message(STATUS "Found Clang")
message(STATUS "Using ClangConfig.cmake in: ${CLANG_CMAKE_DIR}")

add_definitions(${LLVM_DEFINITIONS})
add_definitions(${CLANG_DEFINITIONS})

llvm_map_components_to_libnames(llvm_libs)

add_subdirectory(src)

add_subdirectory(apps)

if((CMAKE_PROJECT_NAME STREQUAL demacrofier) AND BUILD_TESTING)
  add_subdirectory(tests)
endif()
