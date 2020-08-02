add_executable(demacrofy_test
    main_test.cpp
    FileManager_test.cpp
    Parser_test.cpp
    Demacro_test.cpp
)

add_test(NAME demacrofy_test COMMAND demacrofy_test)

target_include_directories(demacrofy_test 
    PRIVATE 
        ${CMAKE_SOURCE_DIR}/include
)

target_link_libraries(demacrofy_test
    PRIVATE
        doctest::doctest
)