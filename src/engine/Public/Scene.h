#pragma once

#include "Components/SpriteComponent.h"
#include "Math/Vec3.h"
#include <memory>
#include <vector>

class Renderer;

class Scene {
public:
    Scene() = default;
    virtual ~Scene() = default;

    virtual void OnUpdate(float deltaTime);
    virtual void OnRender(Renderer& renderer);

    SpriteComponent& AddSprite(const Vec3& position, const Vec3& size,
                               float rotation, std::shared_ptr<Texture> texture,
                               const float color[4],
                               float texOffsetX = 0.0f, float texOffsetY = 0.0f,
                               float texScaleX = 1.0f, float texScaleY = 1.0f);

    void RemoveSprite(size_t index);
    void ClearSprites();
    size_t GetSpriteCount() const { return m_sprites.size(); }
    SpriteComponent* GetSprite(size_t index);

private:
    std::vector<SpriteComponent> m_sprites;
};

class SceneManager {
public:
    static SceneManager& GetInstance();

    void PushScene(std::unique_ptr<Scene> scene);
    void PopScene();
    Scene* GetCurrentScene();
    bool IsEmpty() const;

private:
    SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::vector<std::unique_ptr<Scene>> m_sceneStack;
};
