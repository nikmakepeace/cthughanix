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

add_executable(palette_transition_test
    unit/PaletteTransitionTest.cc
    ${CMAKE_SOURCE_DIR}/src/PaletteTransition.cc
    ${CMAKE_SOURCE_DIR}/src/FramePalette.cc
    ${CMAKE_SOURCE_DIR}/src/ColorPalette.cc
)

target_include_directories(palette_transition_test PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(palette_transition_test PRIVATE m)

add_test(NAME palette_transition_test COMMAND palette_transition_test)

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

add_executable(effect_preset_catalog_test
    unit/EffectPresetCatalogTest.cc
    ${CMAKE_SOURCE_DIR}/src/EffectPresetCatalog.cc
    ${CMAKE_SOURCE_DIR}/src/EffectControl.cc
    ${CMAKE_SOURCE_DIR}/src/EffectChoice.cc
    ${CMAKE_SOURCE_DIR}/src/Option.cc
    ${CMAKE_SOURCE_DIR}/src/OptionInt.cc
    ${CMAKE_SOURCE_DIR}/src/imath.cc
)

target_include_directories(effect_preset_catalog_test PRIVATE
    ${CMAKE_BINARY_DIR}
    ${CMAKE_BINARY_DIR}/src
    ${CMAKE_SOURCE_DIR}/src
)

target_link_libraries(effect_preset_catalog_test PRIVATE m)

add_test(NAME effect_preset_catalog_test COMMAND effect_preset_catalog_test)

if(CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    add_library(format_warning_clean OBJECT
        ${CMAKE_SOURCE_DIR}/src/AutoChanger.cc
        ${CMAKE_SOURCE_DIR}/src/CthughaDisplay.cc
        ${CMAKE_SOURCE_DIR}/src/EffectChoiceLoader.cc
        ${CMAKE_SOURCE_DIR}/src/EffectControl.cc
        ${CMAKE_SOURCE_DIR}/src/Interface.cc
        ${CMAKE_SOURCE_DIR}/src/InterfaceList.cc
        ${CMAKE_SOURCE_DIR}/src/Screenshot.cc
        ${CMAKE_SOURCE_DIR}/src/flames.cc
        ${CMAKE_SOURCE_DIR}/src/info_title_usage.cc
        ${CMAKE_SOURCE_DIR}/src/misc.cc
        ${CMAKE_SOURCE_DIR}/src/palettes.cc
    )

    target_compile_definitions(format_warning_clean PRIVATE
        CTH_LIBDIR="${CTH_DATA_DIR}"
    )

    target_include_directories(format_warning_clean PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/src
        ${CMAKE_SOURCE_DIR}/src
        ${X11_INCLUDE_DIR}
        ${PULSE_INCLUDE_DIRS}
    )

    target_compile_options(format_warning_clean PRIVATE
        -Werror=deprecated-declarations
    )

    add_test(NAME format_warning_clean_build
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target format_warning_clean
    )

    add_library(xaw_warning_clean OBJECT
        ${CMAKE_SOURCE_DIR}/src/DisplayDeviceX11-Panel.cc
    )

    target_compile_definitions(xaw_warning_clean PRIVATE
        CTH_LIBDIR="${CTH_DATA_DIR}"
    )

    target_include_directories(xaw_warning_clean PRIVATE
        ${CMAKE_BINARY_DIR}
        ${CMAKE_BINARY_DIR}/src
        ${CMAKE_SOURCE_DIR}/src
        ${X11_INCLUDE_DIR}
    )

    target_compile_options(xaw_warning_clean PRIVATE
        -Werror=writable-strings
    )

    add_test(NAME xaw_warning_clean_build
        COMMAND ${CMAKE_COMMAND} --build ${CMAKE_BINARY_DIR} --target xaw_warning_clean
    )

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
