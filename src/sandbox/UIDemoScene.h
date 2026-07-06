#pragma once

#include <Scene.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Button.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/UISystem.h>

class UIDemoScene : public Scene {
public:
    UIDemoScene();
    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;
    void SetWindowSize(int w, int h);

private:
    void SetupUI();

    CanvasPanel* m_canvas = nullptr;
    Button* m_testButton = nullptr;
    Label* m_statusLabel = nullptr;
    int m_clickCount = 0;
    int m_windowWidth = 1280;
    int m_windowHeight = 720;
};
