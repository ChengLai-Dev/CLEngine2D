#pragma once

#include "Math/Vec3.h"
#include "Math/Mat4.h"

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
    void EndScene();

    void DrawQuad(const Vec3& position, const Vec3& size, float rotation,
                  Texture* texture, const float color[4],
                  float texOffsetX = 0.0f, float texOffsetY = 0.0f,
                  float texScaleX = 1.0f, float texScaleY = 1.0f);

    void Flush();

private:
    void CreateBuffers();
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

    std::array<Texture*, MAX_TEXTURE_SLOTS> m_textureSlots = {};
    unsigned int m_textureSlotCount = 0;

    std::unique_ptr<Texture> m_whiteTexture = nullptr;
};
