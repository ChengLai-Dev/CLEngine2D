#include "Render/VertexArray.h"
#include "Render/VertexBuffer.h"
#include "Render/IndexBuffer.h"
#include "Render/BufferLayout.h"

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

    const BufferLayout& layout = vb->GetLayout();
    unsigned int attribIndex = 0;
    for (const auto& element : layout.GetElements()) {
        unsigned int comps = ShaderDataTypeComponentCount(element.Type);
        unsigned int slotCount = 1;
        if (element.Type == ShaderDataType::Mat3 || element.Type == ShaderDataType::Mat4)
            slotCount = comps;

        GLenum glType = ShaderDataTypeToGLType(element.Type);
        unsigned int bytesPerSlot = element.Size / comps;

        for (unsigned int slot = 0; slot < slotCount; ++slot) {
            glEnableVertexAttribArray(attribIndex);
            glVertexAttribPointer(attribIndex, static_cast<int>(comps), glType,
                element.Normalized ? GL_TRUE : GL_FALSE,
                static_cast<int>(layout.GetStride()),
                reinterpret_cast<const void*>(static_cast<uintptr_t>(element.Offset + slot * bytesPerSlot)));
            ++attribIndex;
        }
    }

    glBindVertexArray(0);
}

void VertexArray::SetIndexBuffer(IndexBuffer* ib) {
    glBindVertexArray(m_rendererID);
    ib->Bind();
    m_indexBuffer = ib;
    glBindVertexArray(0);
}
