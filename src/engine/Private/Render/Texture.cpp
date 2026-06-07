#include "Render/Texture.h"
#include "Logger.h"

#include <glad/glad.h>
#include <stb_image.h>
#include <format>

Texture::Texture(const std::string& filepath)
    : m_rendererID(0)
    , m_width(0)
    , m_height(0)
    , m_channels(0) {
    stbi_set_flip_vertically_on_load(1);
    unsigned char* data = stbi_load(filepath.c_str(), &m_width, &m_height, &m_channels, 4);
    if (!data) {
        Logger::Error("Failed to load texture: " + filepath + " - " + stbi_failure_reason());
        return;
    }

    glGenTextures(1, &m_rendererID);
    glBindTexture(GL_TEXTURE_2D, m_rendererID);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, m_width, m_height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    stbi_image_free(data);
    Logger::Info(std::format("Loaded texture: {} ({}x{})", filepath, m_width, m_height));
}

Texture::~Texture() {
    glDeleteTextures(1, &m_rendererID);
}

void Texture::Bind(unsigned int slot) const {
    glActiveTexture(GL_TEXTURE0 + slot);
    glBindTexture(GL_TEXTURE_2D, m_rendererID);
}

void Texture::Unbind() const {
    glBindTexture(GL_TEXTURE_2D, 0);
}
