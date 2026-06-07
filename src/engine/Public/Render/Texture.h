#pragma once

#include <string>

class Texture {
public:
    Texture(const std::string& filepath);
    ~Texture();

    void Bind(unsigned int slot = 0) const;
    void Unbind() const;

    int GetWidth() const { return m_width; }
    int GetHeight() const { return m_height; }
    unsigned int GetRendererID() const { return m_rendererID; }

private:
    unsigned int m_rendererID;
    int m_width;
    int m_height;
    int m_channels;
};
