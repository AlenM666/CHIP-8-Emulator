cmake_minimum_required(VERSION 3.20)

set(CMAKE_C_COMPILER clang)

# =============================================================================
# Project
# =============================================================================
project(CHIP8
    VERSION 0.1.0
    DESCRIPTION "CHIP8"
    LANGUAGES C
)

if(CMAKE_SOURCE_DIR STREQUAL CMAKE_BINARY_DIR)
    message(FATAL_ERROR "Do not build in-source. Use: cmake -B build")
endif()

# =============================================================================
# Global Settings
# =============================================================================
set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_EXTENSIONS OFF)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
set(CMAKE_COLOR_DIAGNOSTICS ON)
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

if(NOT CMAKE_CONFIGURATION_TYPES AND NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

# =============================================================================
# Options
# =============================================================================
option(ENABLE_WARNINGS            "Enable warnings"                  ON)
option(ENABLE_WARNINGS_AS_ERRORS  "Warnings as errors"               OFF)
option(ENABLE_LTO                 "Enable Link Time Optimization"     ON)
option(ENABLE_SANITIZERS          "Enable sanitizers (Debug only)"    OFF)
option(ENABLE_UNITY_BUILD         "Enable unity builds"               OFF)
option(ENABLE_PCH                 "Enable precompiled headers"        OFF)
option(ENABLE_NATIVE_OPTIMIZATION "Enable -march=native"              OFF)

# =============================================================================
# Compiler Cache
# =============================================================================
find_program(CCACHE_PROGRAM  ccache)
find_program(SCCACHE_PROGRAM sccache)

if(CCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${CCACHE_PROGRAM})
elseif(SCCACHE_PROGRAM)
    set(CMAKE_C_COMPILER_LAUNCHER ${SCCACHE_PROGRAM})
endif()

# =============================================================================
# FetchContent
# =============================================================================
include(FetchContent)

set(FETCHCONTENT_QUIET OFF)

# # --------------------- FTXUI ---------------------
# set(FETCHCONTENT_UPDATES_DISCONNECTED TRUE)   # Faster subsequent configures
#
# FetchContent_Declare(ftxui
#     GIT_REPOSITORY https://github.com/ArthurSonzogni/FTXUI.git
#     GIT_TAG v7.0.1          # Use a stable release (check latest on GitHub)
#     GIT_SHALLOW TRUE
# )
#
# FetchContent_MakeAvailable(ftxui)

# ================== Raylib =========================
# # Raylib build options — set BEFORE FetchContent_MakeAvailable
set(BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(BUILD_GAMES    OFF CACHE BOOL "" FORCE)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG        6.0         # Pin to a stable release
    GIT_SHALLOW    TRUE
)
FetchContent_MakeAvailable(raylib)
# ================== Raylib =========================


# =============================================================================
# Sources
# =============================================================================
set(SRC_FILES
    src/main.c
    src/window/window.c
    # src/chip8/chip8.c
    )

foreach(src ${SRC_FILES})
    if(NOT EXISTS "${CMAKE_SOURCE_DIR}/${src}")
        message(AUTHOR_WARNING "Source file not found: ${src}")
    endif()
endforeach()

# =============================================================================
# Target
# =============================================================================
add_executable(${PROJECT_NAME} ${SRC_FILES})

enable_testing()
add_executable(chip8_core_tests
    tests/chip8_core_tests.c
    src/chip8/chip8.c
)
target_include_directories(chip8_core_tests PRIVATE ${PROJECT_SOURCE_DIR}/src)
add_test(NAME chip8_core_tests COMMAND chip8_core_tests)

target_include_directories(${PROJECT_NAME}
    PRIVATE
        ${PROJECT_SOURCE_DIR}/src
        ${PROJECT_SOURCE_DIR}/include
)

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        # ftxui::screen
        # ftxui::dom
        # ftxui::component     # Most useful for interactive monitors
        raylib
)

# find_package(Threads REQUIRED)

# Platform link requirements for Raylib
if(APPLE)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        "-framework IOKit"
        "-framework Cocoa"
        "-framework OpenGL"
    )
elseif(UNIX)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        m dl pthread
    )
elseif(WIN32)
    target_link_libraries(${PROJECT_NAME} PRIVATE
        winmm
    )
endif()

# =============================================================================
# Platform Defines
# =============================================================================
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_WINDOWS)
elseif(APPLE)
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_MAC)
else()
    target_compile_definitions(${PROJECT_NAME} PRIVATE PLATFORM_LINUX)
endif()

# =============================================================================
# Precompiled Headers
# =============================================================================
if(ENABLE_PCH)
    target_precompile_headers(${PROJECT_NAME} PRIVATE
        <raylib.h>
        # <ftxui/component/captured_mouse.hpp>
        # <ftxui/component/component.hpp>
        # <ftxui/component/screen_interactive.hpp>
        # <ftxui/dom/elements.hpp>
    )
endif()

# =============================================================================
# Warnings
# =============================================================================
if(ENABLE_WARNINGS)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE
            /W4 /permissive- /Zc:__cplusplus
        )
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE
            -Wall -Wextra -Wpedantic
            -Wshadow
            -Wconversion
            -Wnull-dereference
            -Wformat=2
            -Wcast-align
            -Wunused
            -fdiagnostics-color=always
        )

        if(CMAKE_C_COMPILER_ID STREQUAL "GNU")
            target_compile_options(${PROJECT_NAME} PRIVATE
                -fdiagnostics-show-template-tree
                -fdiagnostics-path-format=inline-events
                -fdiagnostics-show-option
            )
        endif()

        if(CMAKE_C_COMPILER_ID MATCHES "Clang")
            target_compile_options(${PROJECT_NAME} PRIVATE
                -Wdocumentation
                -Wthread-safety
                -Wcomma
                -Wfloat-equal
                -Wconditional-uninitialized
                -Wsign-conversion
                -Wimplicit-fallthrough
            )
        endif()
    endif()
endif()

if(ENABLE_WARNINGS_AS_ERRORS)
    if(MSVC)
        target_compile_options(${PROJECT_NAME} PRIVATE /WX)
    else()
        target_compile_options(${PROJECT_NAME} PRIVATE -Werror)
    endif()
endif()

# =============================================================================
# Optimization
# =============================================================================
if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:/Od /Zi /RTC1>
        $<$<CONFIG:Release>:/O2 /DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:/O2 /Zi>
    )
else()
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Debug>:-O0 -g3 -fno-omit-frame-pointer -fno-optimize-sibling-calls>
        $<$<CONFIG:Release>:-O3 -DNDEBUG>
        $<$<CONFIG:RelWithDebInfo>:-O2 -g>
    )
endif()

if(ENABLE_NATIVE_OPTIMIZATION AND NOT MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE
        $<$<CONFIG:Release>:-march=native>
    )
endif()

# =============================================================================
# Link Time Optimization
# =============================================================================

if(ENABLE_LTO AND CMAKE_BUILD_TYPE STREQUAL "Release")
    include(CheckIPOSupported)
    check_ipo_supported(RESULT supported OUTPUT error LANGUAGES C)
    if(supported)
        set_target_properties(${PROJECT_NAME} PROPERTIES
            INTERPROCEDURAL_OPTIMIZATION_RELEASE TRUE
        )
    else()
        message(WARNING "LTO not supported: ${error}")
    endif()
endif()

# =============================================================================
# Sanitizers
# =============================================================================
# if(ENABLE_SANITIZERS AND NOT MSVC)
#     target_compile_options(${PROJECT_NAME} PRIVATE
#         $<$<CONFIG:Debug>:-fsanitize=address,undefined>
#     )
#     target_link_options(${PROJECT_NAME} PRIVATE
#         $<$<CONFIG:Debug>:-fsanitize=address,undefined>
#     )
# endif()

if(ENABLE_SANITIZERS AND NOT MSVC AND CMAKE_BUILD_TYPE STREQUAL "Debug")
    message(STATUS "│  !! Sanitizers ON — requires libasan + libubsan")
    target_compile_options(${PROJECT_NAME} PRIVATE -fsanitize=address,undefined)
    target_link_options(${PROJECT_NAME}    PRIVATE -fsanitize=address,undefined)
endif()

# =============================================================================
# Assets
# =============================================================================
if(EXISTS "${PROJECT_SOURCE_DIR}/assets")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
        COMMENT "Copying assets..."
    )
endif()

if(EXISTS "${PROJECT_SOURCE_DIR}/data")
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${PROJECT_SOURCE_DIR}/data
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/data
        COMMENT "Copying data files..."
    )
endif()

# =============================================================================
# Install
# =============================================================================
install(TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin)

# =============================================================================
# Summary
# =============================================================================
message(STATUS "")
message(STATUS "┌─ ${PROJECT_NAME} v${PROJECT_VERSION} ─────────────────────")
message(STATUS "│  Build type : ${CMAKE_BUILD_TYPE}")
message(STATUS "│  C std    : C${CMAKE_C_STANDARD}")
message(STATUS "│  Compiler   : ${CMAKE_C_COMPILER_ID} ${CMAKE_C_COMPILER_VERSION}")
message(STATUS "│  Output     : ${CMAKE_RUNTIME_OUTPUT_DIRECTORY}")
message(STATUS "│  Raylib     : FetchContent (6.0)")
message(STATUS "│  LTO        : ${ENABLE_LTO}")
message(STATUS "│  Sanitizers : ${ENABLE_SANITIZERS}")
message(STATUS "│  PCH        : ${ENABLE_PCH}")
message(STATUS "└────────────────────────────────────────────────────────────")
message(STATUS "")

