#include "Scene.h"
#include "SceneGraph/Node.h"
#include "SceneGraph/Sprite.h"
#include "SceneGraph/Widget.h"
#include "SceneGraph/UISystem.h"
#include "Render/Texture.h"
#include "Render/Renderer.h"
#include "UI/UISerializer.h"
#include "Logger.h"


Scene::Scene()
    : m_root(std::make_unique<Node>()) {
}

Scene::~Scene() = default;

Node* Scene::GetRoot() const {
    return m_root.get();
}

void Scene::SetRoot(std::unique_ptr<Node> root) {
    m_root = std::move(root);
}

Sprite* Scene::CreateSprite(const Vec3& position, const Vec2& size,
                            std::shared_ptr<Texture> texture,
                            const float color[4],
                            Node* parent) {
    auto sprite = std::make_unique<Sprite>();
    sprite->SetPosition(position);
    sprite->SetContentSize(size);
    sprite->SetTexture(std::move(texture));
    if (color) {
        sprite->SetColor(Vec4(color[0], color[1], color[2], color[3]));
    }

    Sprite* raw = sprite.get();

    Node* attachTo = parent ? parent : m_root.get();
    attachTo->AddChild(std::move(sprite));

    return raw;
}

void Scene::RemoveAllChildren() {
    while (m_root->GetChildCount() > 0) {
        m_root->RemoveChild(m_root->GetChild(0));
    }
}

bool Scene::LoadUI(const std::string& filepath) {
    Node* uiRoot = UISerializer::LoadFromFile(filepath);
    if (!uiRoot) {
        Logger::Error("Scene::LoadUI: failed to load {}", filepath);
        return false;
    }

    m_uiRoot.reset(uiRoot);

    Widget* uiWidget = dynamic_cast<Widget*>(uiRoot);
    if (uiWidget) {
        UISystem::GetInstance().SetUIRoot(uiWidget);
    }

    Logger::Info("Scene::LoadUI: loaded {}", filepath);
    return true;
}

void Scene::OnUpdate(float deltaTime) {
    if (m_root) {
        m_root->OnUpdate(deltaTime);
    }
    if (m_uiRoot) {
        m_uiRoot->OnUpdate(deltaTime);
    }
}

void Scene::OnRender(Renderer& renderer) {
    if (m_root) {
        Mat4 identity = Mat4::Identity();
        m_root->Visit(renderer, identity, 1.0f);
    }
}

Node* Scene::GetUIRoot() const {
    return m_uiRoot.get();
}

SceneManager& SceneManager::GetInstance() {
    static SceneManager instance;
    return instance;
}

void SceneManager::PushScene(std::shared_ptr<Scene> scene) {
    m_sceneStack.push_back(std::move(scene));
    Logger::Info("SceneManager: scene pushed (stack size: {})", m_sceneStack.size());
}

void SceneManager::PopScene() {
    if (!m_sceneStack.empty()) {
        m_sceneStack.pop_back();
        Logger::Info("SceneManager: scene popped (stack size: {})", m_sceneStack.size());
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
