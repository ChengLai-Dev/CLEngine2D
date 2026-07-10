#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Mat4.h"
#include "Types/Color.h"

#include <glad/glad.h>

#include <memory>
#include <vector>
#include <array>

class Shader;
class VertexArray;
class VertexBuffer;
class IndexBuffer;
class Texture;
class OrthographicCamera;

struct QuadVertex {
    float Position[3] = {};
    float TexCoord[2] = {};
    float TexIndex = 0.0f;
    float Color[4] = {};
};

class Renderer {
public:
    static const unsigned int MAX_TEXTURE_SLOTS = 16;

    static bool InitGL();

    Renderer(unsigned int initialQuadCapacity = 1000);
    ~Renderer();

    void Init();
    void Shutdown();

    void BeginScene(const OrthographicCamera& camera);
    void BeginScene(const Mat4& viewProjection);
    void EndScene();

    void DrawQuad(const Vec3& position, const Vec3& size, float rotation,
                  const Color& color = Color(),
                  Texture* texture = nullptr,
                  float texOffsetX = 0.0f, float texOffsetY = 0.0f,
                  float texScaleX = 1.0f, float texScaleY = 1.0f);

    void DrawQuad(const Mat4& worldTransform, const Vec2& contentSize,
                  const Color& color = Color(),
                  Texture* texture = nullptr,
                  float texOffsetX = 0.0f, float texOffsetY = 0.0f,
                  float texScaleX = 1.0f, float texScaleY = 1.0f);

    void DrawLine(const Vec3& from, const Vec3& to, const Color& color);

    void DrawTriangles(const QuadVertex* vertices, unsigned int vertexCount,
                       const unsigned int* indices, unsigned int indexCount,
                       Texture* texture);

    void Flush();

private:
    void CreateBuffers();
    void CreateLineBuffers();
    void ResetBatch();

    unsigned int m_maxQuads;
    unsigned int m_maxVertices;
    unsigned int m_maxIndices;

    std::unique_ptr<Shader> m_shader = nullptr;
    std::unique_ptr<VertexArray> m_vertexArray = nullptr;
    std::unique_ptr<VertexBuffer> m_vertexBuffer = nullptr;
    std::unique_ptr<IndexBuffer> m_indexBuffer = nullptr;

    std::vector<QuadVertex> m_quadVertexBuffer;
    unsigned int m_quadCount = 0;

    unsigned int m_maxLines;
    unsigned int m_maxLineVertices;
    std::vector<QuadVertex> m_lineVertexBuffer;
    unsigned int m_lineCount = 0;
    std::unique_ptr<VertexArray> m_lineVAO = nullptr;
    std::unique_ptr<VertexBuffer> m_lineVBO = nullptr;

    std::array<Texture*, MAX_TEXTURE_SLOTS> m_textureSlots = {};
    unsigned int m_textureSlotCount = 0;

    std::unique_ptr<Texture> m_whiteTexture = nullptr;

    // Mesh drawing (for DragonBones / Spine)
    std::unique_ptr<VertexArray> m_meshVAO = nullptr;
    std::unique_ptr<VertexBuffer> m_meshVBO = nullptr;
    GLuint m_meshIBO_id = 0;
    unsigned int m_meshVBOCapacity = 0;
};
