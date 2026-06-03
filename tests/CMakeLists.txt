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

add_executable(wave_object_test
    unit/WaveObjectTest.cc
    ${CMAKE_SOURCE_DIR}/src/WaveObject.cc
    ${CMAKE_SOURCE_DIR}/src/EffectChoice.cc
    ${CMAKE_SOURCE_DIR}/src/Option.cc
    ${CMAKE_SOURCE_DIR}/src/OptionInt.cc
    ${CMAKE_SOURCE_DIR}/src/imath.cc
)

target_include_directories(wave_object_test PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(wave_object_test PRIVATE m)

add_test(NAME wave_object_test COMMAND wave_object_test)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_library(waves_warning_clean OBJECT
        ${CMAKE_SOURCE_DIR}/src/waves.cc
    )

    target_compile_definitions(waves_warning_clean PRIVATE
        CTH_LIBDIR="${CTH_DATA_DIR}"
    )

    target_include_directories(waves_warning_clean PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/src
        ${CMAKE_SOURCE_DIR}/src
    )

    target_compile_options(waves_warning_clean PRIVATE
        -Werror
    )

    add_test(NAME waves_warning_clean_build
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target waves_warning_clean
    )
endif()
