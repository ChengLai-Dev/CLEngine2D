#pragma once

#include <glad/glad.h>
#include "BufferLayout.h"

class VertexBuffer {
public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void SetData(const void* data, unsigned int size);

    void SetLayout(const BufferLayout& layout) { m_layout = layout; }
    const BufferLayout& GetLayout() const { return m_layout; }

    GLuint GetRendererID() const { return m_rendererID; }

private:
    GLuint m_rendererID = 0;
    BufferLayout m_layout;
};
