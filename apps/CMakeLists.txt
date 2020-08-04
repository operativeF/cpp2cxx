add_executable(suggest MainSuggest.cpp)

target_link_libraries(suggest
    PRIVATE
        cpp2cxx-core
        Boost::program_options
        Boost::system
    )

target_include_directories(suggest PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    )

add_executable(finalize MainFinalize.cpp)

target_link_libraries(finalize
    PRIVATE
        cpp2cxx-core
        Boost::program_options
        Boost::system
    )

target_include_directories(finalize PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    )
    