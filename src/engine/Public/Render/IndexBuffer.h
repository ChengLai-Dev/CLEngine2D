#pragma once

#include <glad/glad.h>

class IndexBuffer {
public:
    IndexBuffer(const unsigned int* data, unsigned int count);
    ~IndexBuffer();

    void Bind() const;
    void Unbind() const;

    unsigned int GetCount() const { return m_count; }

private:
    GLuint m_rendererID = 0;
    unsigned int m_count;
};
