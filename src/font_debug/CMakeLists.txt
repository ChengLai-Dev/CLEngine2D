add_executable(font_debug main.cpp)
target_include_directories(font_debug PRIVATE
    ${CMAKE_SOURCE_DIR}/third_party/stb_truetype
    ${CMAKE_SOURCE_DIR}/third_party/glfw/deps
)
target_copy_assets(font_debug)
