#pragma once

#include <memory>
#include <string>
#include <unordered_map>

class Texture;
class Shader;

class AssetManager {
public:
    static AssetManager& GetInstance();

    std::shared_ptr<Texture> LoadTexture(const std::string& filepath);
    std::shared_ptr<Texture> GetTexture(const std::string& filepath) const;
    bool HasTexture(const std::string& filepath) const;
    void UnloadTexture(const std::string& filepath);
    void UnloadAllTextures();

    std::shared_ptr<Shader> LoadShader(const std::string& filepath);
    std::shared_ptr<Shader> GetShader(const std::string& filepath) const;
    bool HasShader(const std::string& filepath) const;
    void UnloadShader(const std::string& filepath);
    void UnloadAllShaders();

    void Clear();

private:
    AssetManager() = default;
    ~AssetManager();
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

    std::unordered_map<std::string, std::shared_ptr<Texture>> m_textures;
    std::unordered_map<std::string, std::shared_ptr<Shader>> m_shaders;
};
