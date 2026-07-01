#pragma once

#include <glad/glad.h>
#include <string>

class Texture {
public:
    Texture(const std::string& filepath);
    Texture(unsigned int width, unsigned int height, const unsigned char* data);
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    GLuint GetRendererID() const { return m_rendererID; }

private:
    GLuint m_rendererID = 0;
    int m_width = 0;
    int m_height = 0;
    int m_channels = 0;
};
