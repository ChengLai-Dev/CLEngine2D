#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include <string>
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

    // UI 容器：持有全部 .cui 子树（LoadUI 替换全部 / AddUI 追加叠加）
    Node* GetUIRoot() const;

    Sprite* CreateSprite(const Vec3& position, const Vec2& size,
                         std::shared_ptr<Texture> texture,
                         const float color[4] = nullptr,
                         Node* parent = nullptr);

    void RemoveAllChildren();

    // 加载 .cui 并替换容器内全部子树（历史语义：加载即换整棵 UI 树）
    bool LoadUI(const std::string& filepath);

    // 加载 .cui 追加为容器子树（多 .cui 叠加，不销毁已有树）；
    // 返回新树根（reference，随 Scene/容器销毁）；失败返回 nullptr
    Node* AddUI(const std::string& filepath);

    // 从容器摘除指定子树；成功返回 true。摘除后外部对该子树根的引用即悬垂
    bool RemoveUI(Node* root);

private:
    // 惰性创建 UI 容器（Widget，1280x720，不参与命中）并设为 UISystem 根
    void EnsureUIContainer();

    std::unique_ptr<Node> m_root;
    std::unique_ptr<Node> m_uiRoot;
    bool m_uiContainerReady = false;
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
