add_executable(suggest MainSuggest.cpp)

target_link_libraries(suggest
    PUBLIC
        cpp2cxx-core
        ASTConsumer
        cpp2cxx-debug
        Boost::program_options
        Boost::system
        Boost::wave
        ${llvm_libs}
    )

target_include_directories(suggest PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    )

add_executable(finalize MainFinalize.cpp)

target_link_libraries(finalize
    PUBLIC
        cpp2cxx-core
        ASTConsumer
        cpp2cxx-debug
        Boost::program_options
        Boost::system
        Boost::wave
        ${llvm_libs}
    )

target_include_directories(finalize PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    )