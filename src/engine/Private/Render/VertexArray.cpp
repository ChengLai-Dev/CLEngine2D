#include "Render/VertexArray.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"

#include <glad/glad.h>

VertexArray::VertexArray() {
    glGenVertexArrays(1, &m_rendererID);
}

VertexArray::~VertexArray() {
    glDeleteVertexArrays(1, &m_rendererID);
}

void VertexArray::Bind() const {
    glBindVertexArray(m_rendererID);
}

void VertexArray::Unbind() const {
    glBindVertexArray(0);
}

void VertexArray::AddVertexBuffer(VertexBuffer* vb) {
    m_vertexBuffers.push_back(vb);

    glBindVertexArray(m_rendererID);
    vb->Bind();

    unsigned int index = 0;
    unsigned int offset = 0;

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 3, GL_FLOAT, GL_FALSE, 10 * sizeof(float),
        reinterpret_cast<const void*>(static_cast<size_t>(offset)));
    offset += 3 * sizeof(float);
    ++index;

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float),
        reinterpret_cast<const void*>(static_cast<size_t>(offset)));
    offset += 2 * sizeof(float);
    ++index;

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 1, GL_FLOAT, GL_FALSE, 10 * sizeof(float),
        reinterpret_cast<const void*>(static_cast<size_t>(offset)));
    offset += 1 * sizeof(float);
    ++index;

    glEnableVertexAttribArray(index);
    glVertexAttribPointer(index, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float),
        reinterpret_cast<const void*>(static_cast<size_t>(offset)));

    glBindVertexArray(0);
}

void VertexArray::SetIndexBuffer(IndexBuffer* ib) {
    glBindVertexArray(m_rendererID);
    ib->Bind();
    m_indexBuffer = ib;
    glBindVertexArray(0);
}
