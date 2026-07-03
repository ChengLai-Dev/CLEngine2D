cmake_minimum_required(VERSION 3.16)
project(CLEngine2D VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(MSVC)
    add_compile_options(/utf-8 /W4 /WX-)
    add_compile_options("$<$<CONFIG:Debug>:/Od;/Zi>")
    add_compile_options("$<$<CONFIG:Release>:/O2>")
endif()

# GLFW (local copy)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(third_party/glfw)

# Helper: copy assets to a target's output directory
function(target_copy_assets target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "$<TARGET_FILE_DIR:${target_name}>/assets"
    )
endfunction()

# Helper: copy scripts to a target's output directory
function(target_copy_scripts target_name)
    add_custom_command(TARGET ${target_name} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/scripts"
        "$<TARGET_FILE_DIR:${target_name}>/scripts"
    )
endfunction()

# Libraries
add_subdirectory(third_party/glad)
add_subdirectory(third_party/stb_image)

# Python bindings (optional, for Phase 9)
option(CLENGINE_BUILD_PYTHON "Build Python bindings and py_sandbox" ON)
if(CLENGINE_BUILD_PYTHON)
    include(FetchContent)
    FetchContent_Declare(
        pybind11
        GIT_REPOSITORY https://github.com/pybind/pybind11.git
        GIT_TAG v2.13.6
    )
    set(PYBIND11_FINDPYTHON ON)
    FetchContent_MakeAvailable(pybind11)
    if(NOT pybind11_FOUND)
        message(WARNING "pybind11 not available — Python bindings will be skipped")
        set(CLENGINE_BUILD_PYTHON OFF)
    endif()
endif()

add_subdirectory(src/engine)

# Sandbox (startup project)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sandbox)
add_subdirectory(src/sandbox)

# UI Editor
add_subdirectory(src/editor)

# Font Debug Tool
add_subdirectory(src/font_debug)

# Python Sandbox (requires pybind11)
if(CLENGINE_BUILD_PYTHON)
    add_subdirectory(src/py_sandbox)
endif()
