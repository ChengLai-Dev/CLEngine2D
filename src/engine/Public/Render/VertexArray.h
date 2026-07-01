#pragma once

#include <glad/glad.h>
#include <vector>

class VertexBuffer;
class IndexBuffer;

class VertexArray {
public:
    VertexArray();
    ~VertexArray();

    void Bind() const;
    void Unbind() const;

    void AddVertexBuffer(VertexBuffer* vb);
    void SetIndexBuffer(IndexBuffer* ib);

    IndexBuffer* GetIndexBuffer() const { return m_indexBuffer; }

private:
    GLuint m_rendererID = 0;
    std::vector<VertexBuffer*> m_vertexBuffers;
    IndexBuffer* m_indexBuffer = nullptr;
};
