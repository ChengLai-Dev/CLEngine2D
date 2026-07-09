#include "Render/Renderer.h"
#include "Render/Shader.h"
#include "Render/VertexArray.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/Texture.h"
#include "Render/OrthographicCamera.h"
#include "Render/BufferLayout.h"
#include "Platform/Window.h"
#include "Utils.h"
#include "Logger.h"

#include <glad/glad.h>

bool Renderer::InitGL() {
    if (!gladLoadGLLoader((GLADloadproc)Window::GetProcAddress)) {
        Logger::Fatal("Failed to initialize Glad");
        return false;
    }

    Logger::Info("OpenGL {}.{} loaded", GLVersion.major, GLVersion.minor);
    Logger::Info("Renderer: {}", (const char*)glGetString(GL_RENDERER));
    Logger::Info("Vendor: {}", (const char*)glGetString(GL_VENDOR));
    Logger::Info("GLSL Version: {}", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

    if (IS_DEBUG) {
        if (GLAD_GL_VERSION_4_3 || GLAD_GL_KHR_debug) {
            glEnable(GL_DEBUG_OUTPUT);
            glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);
            glDebugMessageCallback([](GLenum source, GLenum type, GLuint id,
                                      GLenum severity, GLsizei length,
                                      const GLchar* message, const void* userParam) {
                // NVIDIA: 着色器特化编译提示（良性，仅在首次绘制时触发一次）
                // if (id == 131218) return;

                if (severity == GL_DEBUG_SEVERITY_HIGH) {
                    if (type == GL_DEBUG_TYPE_ERROR) {
                        Logger::Error("[GL] {}", message);
                        __debugbreak();
                    } else {
                        Logger::Warn("[GL] {}", message);
                    }
                } else if (severity == GL_DEBUG_SEVERITY_MEDIUM) {
                    Logger::Warn("[GL] {}", message);
                } else if (severity == GL_DEBUG_SEVERITY_LOW) {
                    Logger::Info("[GL] {}", message);
                } else {
                    Logger::Debug("[GL] {}", message);
                }
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
    , m_maxLines(initialQuadCapacity)
    , m_maxLineVertices(initialQuadCapacity * 2) {
}

Renderer::~Renderer() {
    Shutdown();
}

void Renderer::Init() {
    Logger::Info("Initializing renderer...");

    m_quadVertexBuffer.resize(m_maxVertices);
    CreateBuffers();

    m_lineVertexBuffer.resize(m_maxLineVertices);
    CreateLineBuffers();

    unsigned char whitePixel[4] = { 255, 255, 255, 255 };
    m_whiteTexture = std::unique_ptr<Texture>(new Texture(1, 1, whitePixel));

    m_vertexArray->Bind();
    m_shader = std::unique_ptr<Shader>(new Shader("assets/shaders/Texture.glsl"));
    m_shader->Bind();

    int samplers[MAX_TEXTURE_SLOTS];
    for (unsigned int i = 0; i < MAX_TEXTURE_SLOTS; ++i) {
        samplers[i] = static_cast<int>(i);
    }
    m_shader->SetIntArray("u_Textures", samplers, MAX_TEXTURE_SLOTS);

    Logger::Info("Renderer initialized");
}

void Renderer::CreateBuffers() {
    m_vertexArray = std::unique_ptr<VertexArray>(new VertexArray());
    m_vertexBuffer = std::unique_ptr<VertexBuffer>(new VertexBuffer(nullptr, m_maxVertices * sizeof(QuadVertex)));

    BufferLayout quadLayout = {
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float2, "a_TexCoord" },
        { ShaderDataType::Float,  "a_TexIndex" },
        { ShaderDataType::Float4, "a_Color" }
    };
    m_vertexBuffer->SetLayout(quadLayout);

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

void Renderer::CreateLineBuffers() {
    m_lineVAO = std::unique_ptr<VertexArray>(new VertexArray());
    m_lineVBO = std::unique_ptr<VertexBuffer>(new VertexBuffer(nullptr, m_maxLineVertices * sizeof(QuadVertex)));

    BufferLayout lineLayout = {
        { ShaderDataType::Float3, "a_Position" },
        { ShaderDataType::Float2, "a_TexCoord" },
        { ShaderDataType::Float,  "a_TexIndex" },
        { ShaderDataType::Float4, "a_Color" }
    };
    m_lineVBO->SetLayout(lineLayout);
    m_lineVAO->AddVertexBuffer(m_lineVBO.get());
}

void Renderer::Shutdown() {
    m_whiteTexture.reset();
    m_shader.reset();
    m_vertexArray.reset();
    m_vertexBuffer.reset();
    m_indexBuffer.reset();
    m_quadVertexBuffer.clear();
    m_lineVAO.reset();
    m_lineVBO.reset();
    m_lineVertexBuffer.clear();
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
                        const Color& color, Texture* texture,
                        float texOffsetX, float texOffsetY,
                        float texScaleX, float texScaleY) {
    Mat4 transform =
        Mat4::Translate(position) *
        Mat4::RotateZ(rotation) *
        Mat4::Scale(size);

    DrawQuad(transform, Vec2(size.x, size.y),
             color, texture,
             texOffsetX, texOffsetY,
             texScaleX, texScaleY);
}

void Renderer::DrawQuad(const Mat4& worldTransform, const Vec2& contentSize,
                        const Color& color, Texture* texture,
                        float texOffsetX, float texOffsetY,
                        float texScaleX, float texScaleY) {
    if (m_quadCount >= m_maxQuads) {
        Flush();
    }

    if (!texture) {
        texture = m_whiteTexture.get();
    }

    unsigned int texIndex = 0;
    {
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

    float halfW = contentSize.x * 0.5f;
    float halfH = contentSize.y * 0.5f;

    Vec3 localVerts[4] = {
        Vec3(-halfW, -halfH, 0.0f),
        Vec3( halfW, -halfH, 0.0f),
        Vec3( halfW,  halfH, 0.0f),
        Vec3(-halfW,  halfH, 0.0f)
    };

    Vec3 worldVerts[4];
    for (int i = 0; i < 4; ++i) {
        worldVerts[i] = worldTransform.TransformPoint(localVerts[i]);
    }

    float uvs[4][2] = {
        { texOffsetX,               texOffsetY + texScaleY },
        { texOffsetX + texScaleX,   texOffsetY + texScaleY },
        { texOffsetX + texScaleX,   texOffsetY },
        { texOffsetX,               texOffsetY }
    };

    unsigned int start = m_quadCount * 4;
    for (unsigned int i = 0; i < 4; ++i) {
        m_quadVertexBuffer[start + i].Position[0] = worldVerts[i].x;
        m_quadVertexBuffer[start + i].Position[1] = worldVerts[i].y;
        m_quadVertexBuffer[start + i].Position[2] = worldVerts[i].z;
        m_quadVertexBuffer[start + i].TexCoord[0] = uvs[i][0];
        m_quadVertexBuffer[start + i].TexCoord[1] = uvs[i][1];
        m_quadVertexBuffer[start + i].TexIndex = static_cast<float>(texIndex);
        m_quadVertexBuffer[start + i].Color[0] = color.r;
        m_quadVertexBuffer[start + i].Color[1] = color.g;
        m_quadVertexBuffer[start + i].Color[2] = color.b;
        m_quadVertexBuffer[start + i].Color[3] = color.a;
    }

    ++m_quadCount;
}

void Renderer::DrawLine(const Vec3& from, const Vec3& to, const Color& color) {
    if (m_lineCount >= m_maxLines) {
        Flush();
    }

    unsigned int start = m_lineCount * 2;
    for (unsigned int i = 0; i < 2; ++i) {
        Vec3 p = (i == 0) ? from : to;
        m_lineVertexBuffer[start + i].Position[0] = p.x;
        m_lineVertexBuffer[start + i].Position[1] = p.y;
        m_lineVertexBuffer[start + i].Position[2] = p.z;
        m_lineVertexBuffer[start + i].TexCoord[0] = 0.0f;
        m_lineVertexBuffer[start + i].TexCoord[1] = 0.0f;
        m_lineVertexBuffer[start + i].TexIndex = 0.0f;
        m_lineVertexBuffer[start + i].Color[0] = color.r;
        m_lineVertexBuffer[start + i].Color[1] = color.g;
        m_lineVertexBuffer[start + i].Color[2] = color.b;
        m_lineVertexBuffer[start + i].Color[3] = color.a;
    }

    ++m_lineCount;
}

void Renderer::DrawTriangles(const QuadVertex* vertices, unsigned int vertexCount,
                              const unsigned int* indices, unsigned int indexCount,
                              Texture* texture) {
    Flush();

    if (!m_meshVAO) {
        m_meshVAO = std::unique_ptr<VertexArray>(new VertexArray());
        m_meshVAO->Bind();

        m_meshVBO = std::unique_ptr<VertexBuffer>(new VertexBuffer(nullptr, 0));
        BufferLayout quadLayout = {
            { ShaderDataType::Float3, "a_Position" },
            { ShaderDataType::Float2, "a_TexCoord" },
            { ShaderDataType::Float,  "a_TexIndex" },
            { ShaderDataType::Float4, "a_Color" }
        };
        m_meshVBO->SetLayout(quadLayout);
        m_meshVAO->AddVertexBuffer(m_meshVBO.get());

        glGenBuffers(1, &m_meshIBO_id);
        m_meshVAO->Unbind();
    }

    unsigned int dataSize = vertexCount * sizeof(QuadVertex);
    if (dataSize > m_meshVBOCapacity) {
        m_meshVBOCapacity = dataSize;
        m_meshVBO->Bind();
        glBufferData(GL_ARRAY_BUFFER, static_cast<long long>(dataSize), vertices, GL_DYNAMIC_DRAW);
        m_meshVBO->Unbind();
    } else {
        m_meshVBO->Bind();
        glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<long long>(dataSize), vertices);
        m_meshVBO->Unbind();
    }

    unsigned int indexDataSize = indexCount * sizeof(unsigned int);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshIBO_id);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, static_cast<long long>(indexDataSize), indices, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    texture->Bind(0);
    m_shader->SetInt("u_Textures[0]", 0);

    m_meshVAO->Bind();
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_meshIBO_id);
    glDrawElements(GL_TRIANGLES, static_cast<int>(indexCount), GL_UNSIGNED_INT, nullptr);
    m_meshVAO->Unbind();
}

void Renderer::Flush() {
    if (m_quadCount > 0) {
        unsigned int dataSize = m_quadCount * 4 * sizeof(QuadVertex);
        m_vertexBuffer->SetData(m_quadVertexBuffer.data(), dataSize);

        for (unsigned int i = 0; i < m_textureSlotCount; ++i) {
            m_textureSlots[i]->Bind(i);
        }

        m_vertexArray->Bind();
        unsigned int indexCount = m_quadCount * 6;
        glDrawElements(GL_TRIANGLES, static_cast<int>(indexCount), GL_UNSIGNED_INT, nullptr);
        m_vertexArray->Unbind();
    }

    if (m_lineCount > 0) {
        unsigned int lineDataSize = m_lineCount * 2 * sizeof(QuadVertex);
        m_lineVBO->SetData(m_lineVertexBuffer.data(), lineDataSize);

        m_whiteTexture->Bind(0);
        m_shader->SetInt("u_Textures[0]", 0);

        m_lineVAO->Bind();
        glDrawArrays(GL_LINES, 0, static_cast<int>(m_lineCount * 2));
        m_lineVAO->Unbind();
    }

    ResetBatch();
}

void Renderer::ResetBatch() {
    m_quadCount = 0;
    m_lineCount = 0;
    m_textureSlotCount = 0;
    m_textureSlots.fill(nullptr);
}
