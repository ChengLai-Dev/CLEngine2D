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

# Libraries
add_subdirectory(third_party/glad)
add_subdirectory(third_party/stb_image)
add_subdirectory(src/engine)

# Sandbox (startup project)
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT sandbox)
add_subdirectory(src/sandbox)

# UI Editor
add_subdirectory(src/editor)
