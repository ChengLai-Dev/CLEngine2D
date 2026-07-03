#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include <memory>
#include <vector>

class Node;
class Sprite;
class Renderer;
class Texture;

class Scene {
public:
    Scene();
    virtual ~Scene();

    virtual void OnUpdate(float deltaTime);
    virtual void OnRender(Renderer& renderer);

    Node* GetRoot() const;
    void SetRoot(std::unique_ptr<Node> root);

    Sprite* CreateSprite(const Vec3& position, const Vec2& size,
                         std::shared_ptr<Texture> texture,
                         const float color[4] = nullptr,
                         Node* parent = nullptr);

    void RemoveAllChildren();

private:
    std::unique_ptr<Node> m_root;
};

class SceneManager {
public:
    static SceneManager& GetInstance();

    void PushScene(std::shared_ptr<Scene> scene);
    void PopScene();
    Scene* GetCurrentScene();
    bool IsEmpty() const;

private:
    SceneManager() = default;
    SceneManager(const SceneManager&) = delete;
    SceneManager& operator=(const SceneManager&) = delete;

    std::vector<std::shared_ptr<Scene>> m_sceneStack;
};
