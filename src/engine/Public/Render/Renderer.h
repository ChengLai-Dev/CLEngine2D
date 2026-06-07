#pragma once

#include "Vec3.h"
#include "Mat4.h"

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
    float Position[3];
    float TexCoord[2];
    float TexIndex;
    float Color[4];
};

class Renderer {
public:
    static const unsigned int MAX_TEXTURE_SLOTS = 16;

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

    std::unique_ptr<Shader> m_shader;
    std::unique_ptr<VertexArray> m_vertexArray;
    std::unique_ptr<VertexBuffer> m_vertexBuffer;
    std::unique_ptr<IndexBuffer> m_indexBuffer;

    std::vector<QuadVertex> m_quadVertexBuffer;
    unsigned int m_quadCount;

    std::array<Texture*, MAX_TEXTURE_SLOTS> m_textureSlots;
    unsigned int m_textureSlotCount;
};
