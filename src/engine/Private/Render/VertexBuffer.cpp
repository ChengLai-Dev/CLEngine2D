#include "Render/VertexBuffer.h"
#include <glad/glad.h>

VertexBuffer::VertexBuffer(const void* data, unsigned int size) {
    glGenBuffers(1, &m_rendererID);
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferData(GL_ARRAY_BUFFER, static_cast<long long>(size), data, GL_STREAM_DRAW);
}

VertexBuffer::~VertexBuffer() {
    glDeleteBuffers(1, &m_rendererID);
}

void VertexBuffer::Bind() const {
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
}

void VertexBuffer::Unbind() const {
    glBindBuffer(GL_ARRAY_BUFFER, 0);
}

void VertexBuffer::SetData(const void* data, unsigned int size) {
    glBindBuffer(GL_ARRAY_BUFFER, m_rendererID);
    glBufferSubData(GL_ARRAY_BUFFER, 0, static_cast<long long>(size), data);
}
