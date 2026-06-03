add_executable(indexed_display_frame_test
    unit/IndexedDisplayFrameTest.cc
    ${CMAKE_SOURCE_DIR}/src/IndexedDisplayFrame.cc
)

target_include_directories(indexed_display_frame_test PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(indexed_display_frame_test PRIVATE m)

add_test(NAME indexed_display_frame_test COMMAND indexed_display_frame_test)

add_executable(warning_clean_smoke_test
    unit/WarningCleanSmokeTest.cc
)

target_include_directories(warning_clean_smoke_test PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    target_compile_options(warning_clean_smoke_test PRIVATE
        -Werror=macro-redefined
        -Werror=deprecated-declarations
    )
endif()

add_test(NAME warning_clean_smoke_test COMMAND warning_clean_smoke_test)
