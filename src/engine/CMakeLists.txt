add_library(engine STATIC
    Public/Application.h
    Public/Logger.h
    Public/Utils.h
    Private/Application.cpp
    Private/Logger.cpp
    Public/Platform/Window.h
    Private/Platform/Window.cpp
    Private/Utils.cpp
    Public/Render/Vec3.h
    Public/Render/Mat4.h
    Public/Render/Shader.h
    Private/Render/Shader.cpp
    Public/Render/VertexBuffer.h
    Private/Render/VertexBuffer.cpp
    Public/Render/IndexBuffer.h
    Private/Render/IndexBuffer.cpp
    Public/Render/VertexArray.h
    Private/Render/VertexArray.cpp
    Public/Render/Texture.h
    Private/Render/Texture.cpp
    Public/Render/RenderCommand.h
    Private/Render/RenderCommand.cpp
    Public/Render/OrthographicCamera.h
    Private/Render/OrthographicCamera.cpp
    Public/Render/Renderer.h
    Private/Render/Renderer.cpp
)

target_include_directories(engine PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/Public
)

target_include_directories(engine PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/Private
)

target_link_libraries(engine PUBLIC glfw glad stb_image)
