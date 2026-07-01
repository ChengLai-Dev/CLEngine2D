#include "AssetManager.h"
#include "Render/Texture.h"
#include "Render/Shader.h"
#include "Logger.h"

#include <format>

AssetManager& AssetManager::GetInstance() {
    static AssetManager instance;
    return instance;
}

AssetManager::~AssetManager() {
    Clear();
}

std::shared_ptr<Texture> AssetManager::LoadTexture(const std::string& filepath) {
    auto it = m_textures.find(filepath);
    if (it != m_textures.end()) {
        return it->second;
    }

    auto texture = std::make_shared<Texture>(filepath);
    m_textures[filepath] = texture;
    Logger::Debug(std::format("AssetManager: cached texture '{}'", filepath));
    return texture;
}

std::shared_ptr<Texture> AssetManager::GetTexture(const std::string& filepath) const {
    auto it = m_textures.find(filepath);
    return (it != m_textures.end()) ? it->second : nullptr;
}

bool AssetManager::HasTexture(const std::string& filepath) const {
    return m_textures.find(filepath) != m_textures.end();
}

void AssetManager::UnloadTexture(const std::string& filepath) {
    auto it = m_textures.find(filepath);
    if (it != m_textures.end()) {
        m_textures.erase(it);
        Logger::Debug(std::format("AssetManager: unloaded texture '{}'", filepath));
    }
}

void AssetManager::UnloadAllTextures() {
    m_textures.clear();
    Logger::Debug("AssetManager: all textures unloaded");
}

std::shared_ptr<Shader> AssetManager::LoadShader(const std::string& filepath) {
    auto it = m_shaders.find(filepath);
    if (it != m_shaders.end()) {
        return it->second;
    }

    auto shader = std::make_shared<Shader>(filepath);
    m_shaders[filepath] = shader;
    Logger::Debug(std::format("AssetManager: cached shader '{}'", filepath));
    return shader;
}

std::shared_ptr<Shader> AssetManager::GetShader(const std::string& filepath) const {
    auto it = m_shaders.find(filepath);
    return (it != m_shaders.end()) ? it->second : nullptr;
}

bool AssetManager::HasShader(const std::string& filepath) const {
    return m_shaders.find(filepath) != m_shaders.end();
}

void AssetManager::UnloadShader(const std::string& filepath) {
    auto it = m_shaders.find(filepath);
    if (it != m_shaders.end()) {
        m_shaders.erase(it);
        Logger::Debug(std::format("AssetManager: unloaded shader '{}'", filepath));
    }
}

void AssetManager::UnloadAllShaders() {
    m_shaders.clear();
    Logger::Debug("AssetManager: all shaders unloaded");
}

void AssetManager::Clear() {
    UnloadAllTextures();
    UnloadAllShaders();
    Logger::Debug("AssetManager: all assets cleared");
}
