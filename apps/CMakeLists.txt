add_executable(suggest MainSuggest.cpp)

target_link_libraries(suggest
    PRIVATE
        ASTConsumer
        cpp2cxx-core
        Boost::program_options
        Boost::system
    )

target_include_directories(suggest PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    )

target_precompile_headers(suggest
    PRIVATE
        <algorithm>
        <filesystem>
        <fstream>
        <iostream>
        <map>
        <memory>
        <set>
        <string>
        <vector>
        <boost/program_options.hpp>
        <fmt/core.h>
        <fmt/ostream.h>
        <fmt/ranges.h>
    )

add_executable(finalize MainFinalize.cpp)

target_link_libraries(finalize
    PRIVATE
        ASTConsumer
        cpp2cxx-core
        Boost::program_options
        Boost::system
    )

target_include_directories(finalize PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    )
    
target_precompile_headers(finalize
    REUSE_FROM
        suggest
    )