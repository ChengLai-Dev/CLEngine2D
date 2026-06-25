#pragma once

class VertexBuffer {
public:
    VertexBuffer(const void* data, unsigned int size);
    ~VertexBuffer();

    void Bind() const;
    void Unbind() const;
    void SetData(const void* data, unsigned int size);

    unsigned int GetRendererID() const { return m_rendererID; }

private:
    unsigned int m_rendererID = 0;
};
