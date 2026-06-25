#include "Render/Renderer.h"
#include "Render/Shader.h"
#include "Render/VertexArray.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Texture.h"
#include "Render/OrthographicCamera.h"
#include "Platform/Window.h"
#include "Utils.h"
#include "Logger.h"

#include <glad/glad.h>
#include <format>

bool Renderer::InitGL() {
    if (!gladLoadGLLoader((GLADloadproc)Window::GetProcAddress)) {
        Logger::Fatal("Failed to initialize Glad");
        return false;
    }

    Logger::Info(std::format("OpenGL {}.{} loaded", GLVersion.major, GLVersion.minor));
    Logger::Info("Renderer: " + std::string((const char*)glGetString(GL_RENDERER)));
    Logger::Info("Vendor: " + std::string((const char*)glGetString(GL_VENDOR)));
    Logger::Info("GLSL Version: " + std::string((const char*)glGetString(GL_SHADING_LANGUAGE_VERSION)));

    if (IS_DEBUG) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glDebugMessageCallback([](GLenum source, GLenum type, GLuint id,
                                      GLenum severity, GLsizei length,
                                      const GLchar* message, const void* userParam) {
                Logger::Error(std::format("[GL] {}", message));
                __debugbreak();
            }, nullptr);
            Logger::Info("GL debug output enabled");
        } else {
            Logger::Warn("GL debug output not available");
        }
    }

    return true;
}

Renderer::Renderer(unsigned int initialQuadCapacity)
    : m_maxQuads(initialQuadCapacity)
    , m_maxVertices(initialQuadCapacity * 4)
    , m_maxIndices(initialQuadCapacity * 6)
    , m_shader(nullptr)
    , m_vertexArray(nullptr)
    , m_vertexBuffer(nullptr)
    , m_indexBuffer(nullptr)
{
}

Renderer::~Renderer() {
    Shutdown();
}

void Renderer::Init() {
    Logger::Info("Initializing renderer...");

    m_shader = std::unique_ptr<Shader>(new Shader("assets/shaders/Texture.glsl"));
    m_shader->Bind();

    int samplers[MAX_TEXTURE_SLOTS];
    for (unsigned int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        samplers[i] = static_cast<int>(i);
    }
    m_shader->SetIntArray("u_Textures", samplers, MAX_TEXTURE_SLOTS);

    m_quadVertexBuffer.resize(m_maxVertices);
    CreateBuffers();

    Logger::Info("Renderer initialized");
}

void Renderer::CreateBuffers() {
    m_vertexArray = std::unique_ptr<VertexArray>(new VertexArray());
    m_vertexBuffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(nullptr, m_maxVertices * sizeof(QuadVertex)));

    std::vector<unsigned int> indices(m_maxIndices);
    unsigned int offset = 0;
    for (unsigned int i = 0; i < m_maxIndices; i += 6) {
        indices[i + 0] = offset + 0;
        indices[i + 1] = offset + 1;
        indices[i + 2] = offset + 2;
        indices[i + 3] = offset + 2;
        indices[i + 4] = offset + 3;
        indices[i + 5] = offset + 0;
        offset += 4;
    }

    m_indexBuffer = std::unique_ptr<IndexBuffer>(new IndexBuffer(indices.data(), m_maxIndices));

    m_vertexArray->AddVertexBuffer(m_vertexBuffer.get());
    m_vertexArray->SetIndexBuffer(m_indexBuffer.get());
}

void Renderer::Shutdown() {
    m_shader.reset();
    m_vertexArray.reset();
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_quadVertexBuffer.clear();
}

void Renderer::BeginScene(const OrthographicCamera& camera) {
    m_shader->Bind();
    m_shader->SetMat4("u_ViewProjection", camera.GetViewProjectionMatrix());
    ResetBatch();
}

void Renderer::EndScene() {
    Flush();
}

void Renderer::DrawQuad(const Vec3& position, const Vec3& size, float rotation,
                        Texture* texture, const float color[4],
                        float texOffsetX, float texOffsetY,
                        float texScaleX, float texScaleY) {
    // 当前批次写满了 → 先画出去，清空，继续写
    if (m_quadCount >= m_maxQuads) {
        Flush();
    }

    unsigned int texIndex = 0;
    if (texture) {
        bool found = false;
        for (unsigned int i = 0; i < m_textureSlotCount; ++i) {
            if (m_textureSlots[i] == texture) {
                texIndex = i;
                found = true;
                break;
            }
        }
        if (!found) {
            if (m_textureSlotCount >= MAX_TEXTURE_SLOTS) {
                Flush();
            }
            texIndex = m_textureSlotCount;
            m_textureSlots[m_textureSlotCount++] = texture;
        }
    }

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;

    float cosR = cosf(rotation);
    float sinR = sinf(rotation);

    float x[4], y[4];
    float localX[4] = { -halfW,  halfW,  halfW, -halfW };
    float localY[4] = { -halfH, -halfH,  halfH,  halfH };

    for (int i = 0; i < 4; ++i) {
        x[i] = localX[i] * cosR - localY[i] * sinR + position.x;
        y[i] = localX[i] * sinR + localY[i] * cosR + position.y;
    }

    float uvs[4][2] = {
        { texOffsetX,               texOffsetY + texScaleY },
        { texOffsetX + texScaleX,   texOffsetY + texScaleY },
        { texOffsetX + texScaleX,   texOffsetY },
        { texOffsetX,               texOffsetY }
    };

    unsigned int start = m_quadCount * 4;
    for (unsigned int i = 0; i < 4; ++i) {
        m_quadVertexBuffer[start + i].Position[0] = x[i];
        m_quadVertexBuffer[start + i].Position[1] = y[i];
        m_quadVertexBuffer[start + i].Position[2] = 0.0f;
        m_quadVertexBuffer[start + i].TexCoord[0] = uvs[i][0];
        m_quadVertexBuffer[start + i].TexCoord[1] = uvs[i][1];
        m_quadVertexBuffer[start + i].TexIndex = static_cast<float>(texIndex);
        m_quadVertexBuffer[start + i].Color[0] = color[0];
        m_quadVertexBuffer[start + i].Color[1] = color[1];
        m_quadVertexBuffer[start + i].Color[2] = color[2];
        m_quadVertexBuffer[start + i].Color[3] = color[3];
    }

    ++m_quadCount;
}

void Renderer::Flush() {
    if (m_quadCount == 0) return;

    // CPU 内存 → 上传到 GPU 显存
    unsigned int dataSize = m_quadCount * 4 * sizeof(QuadVertex);
    m_vertexBuffer->SetData(m_quadVertexBuffer.data(), dataSize);

    // 绑定所有用到的纹理
    for (unsigned int i = 0; i < m_textureSlotCount; ++i) {
        m_textureSlots[i]->Bind(i);
    }

    // 执行绘制
    m_vertexArray->Bind();
    unsigned int indexCount = m_quadCount * 6;
    glDrawElements(GL_TRIANGLES, static_cast<int>(indexCount), GL_UNSIGNED_INT, nullptr);
    m_vertexArray->Unbind();

    ResetBatch();
}

void Renderer::ResetBatch() {
    m_quadCount = 0;
    m_textureSlotCount = 0;
    m_textureSlots.fill(nullptr);
}
