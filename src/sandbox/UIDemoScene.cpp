#include "UIDemoScene.h"
#include <Logger.h>
#include <SceneGraph/Node.h>

UIDemoScene::UIDemoScene() {
    SetupUI();
}

void UIDemoScene::SetupUI() {
    auto canvas = std::make_unique<CanvasPanel>();
    m_canvas = canvas.get();

    FAnchorData canvasAnchor;
    canvasAnchor.AnchorMin = Vec2(0.0f, 0.0f);
    canvasAnchor.AnchorMax = Vec2(1.0f, 1.0f);
    canvasAnchor.Size = Vec2(0.0f, 0.0f);
    canvasAnchor.Position = Vec2(0.0f, 0.0f);
    m_canvas->SetContentSize(Vec2(1280.0f, 720.0f));

    auto button = std::make_unique<Button>();
    m_testButton = button.get();
    m_testButton->SetContentSize(Vec2(200.0f, 50.0f));
    m_testButton->SetText("Click Me!");
    m_testButton->SetTextColor(Vec4(1.0f, 1.0f, 1.0f, 1.0f));
    m_testButton->OnClicked([this](Button* btn) {
        m_clickCount++;
        Logger::Info("{}", m_clickCount);
    });

    FAnchorData btnAnchor;
    btnAnchor.AnchorMin = Vec2(0.5f, 0.5f);
    btnAnchor.AnchorMax = Vec2(0.5f, 0.5f);
    btnAnchor.Position = Vec2(-100.0f, -25.0f);
    btnAnchor.Size = Vec2(200.0f, 50.0f);
    canvas->AddChildWithAnchor(std::move(button), btnAnchor);

    auto label = std::make_unique<Label>();
    m_statusLabel = label.get();
    m_statusLabel->SetContentSize(Vec2(400.0f, 40.0f));
    m_statusLabel->SetText("Button clicks: 0");
    m_statusLabel->SetTextColor(Vec4(1.0f, 1.0f, 0.0f, 1.0f));

    FAnchorData labelAnchor;
    labelAnchor.AnchorMin = Vec2(0.5f, 0.5f);
    labelAnchor.AnchorMax = Vec2(0.5f, 0.5f);
    labelAnchor.Position = Vec2(-200.0f, 50.0f);
    labelAnchor.Size = Vec2(400.0f, 40.0f);
    canvas->AddChildWithAnchor(std::move(label), labelAnchor);

    SetRoot(std::move(canvas));

    UISystem::GetInstance().AddLayer(m_canvas, 0);

    Logger::Info("UI Demo Scene initialized");
}

void UIDemoScene::SetWindowSize(int w, int h) {
    m_windowWidth = w;
    m_windowHeight = h;
}

void UIDemoScene::OnUpdate(float deltaTime) {
    Scene::OnUpdate(deltaTime);

    m_canvas->UpdateLayout(Vec2(static_cast<float>(m_windowWidth),
                                static_cast<float>(m_windowHeight)));

    UISystem::GetInstance().ProcessEvents();

    m_statusLabel->SetText("Button clicks: " + std::to_string(m_clickCount));
}

void UIDemoScene::OnRender(Renderer& renderer) {
    Scene::OnRender(renderer);
}
