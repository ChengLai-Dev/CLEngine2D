#include "Scene.h"
#include "Render/Renderer.h"
#include "Logger.h"

#include <format>

void Scene::OnUpdate(float deltaTime) {
    (void)deltaTime;
}

void Scene::OnRender(Renderer& renderer) {
    for (const auto& sprite : m_sprites) {
        renderer.DrawQuad(sprite.Position, sprite.Size, sprite.Rotation,
                          sprite.Texture.get(), sprite.Color,
                          sprite.TexOffsetX, sprite.TexOffsetY,
                          sprite.TexScaleX, sprite.TexScaleY);
    }
}

SpriteComponent& Scene::AddSprite(const Vec3& position, const Vec3& size,
                                   float rotation, std::shared_ptr<Texture> texture,
                                   const float color[4],
                                   float texOffsetX, float texOffsetY,
                                   float texScaleX, float texScaleY) {
    SpriteComponent sprite;
    sprite.Position = position;
    sprite.Size = size;
    sprite.Rotation = rotation;
    sprite.Texture = std::move(texture);
    sprite.Color[0] = color[0];
    sprite.Color[1] = color[1];
    sprite.Color[2] = color[2];
    sprite.Color[3] = color[3];
    sprite.TexOffsetX = texOffsetX;
    sprite.TexOffsetY = texOffsetY;
    sprite.TexScaleX = texScaleX;
    sprite.TexScaleY = texScaleY;
    m_sprites.push_back(std::move(sprite));
    return m_sprites.back();
}

void Scene::RemoveSprite(size_t index) {
    if (index < m_sprites.size()) {
        m_sprites.erase(m_sprites.begin() + static_cast<ptrdiff_t>(index));
    }
}

void Scene::ClearSprites() {
    m_sprites.clear();
}

SpriteComponent* Scene::GetSprite(size_t index) {
    return (index < m_sprites.size()) ? &m_sprites[index] : nullptr;
}

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::PushScene(std::unique_ptr<Scene> scene) {
    m_sceneStack.push_back(std::move(scene));
    Logger::Info(std::format("SceneManager: scene pushed (stack size: {})", m_sceneStack.size()));
}

void SceneManager::PopScene() {
    if (!m_sceneStack.empty()) {
        m_sceneStack.pop_back();
        Logger::Info(std::format("SceneManager: scene popped (stack size: {})", m_sceneStack.size()));
    }
}

Scene* SceneManager::GetCurrentScene() {
    if (m_sceneStack.empty()) {
        return nullptr;
    }
    return m_sceneStack.back().get();
}

bool SceneManager::IsEmpty() const {
    return m_sceneStack.empty();
}
