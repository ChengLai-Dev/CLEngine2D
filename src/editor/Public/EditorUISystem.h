#pragma once

#include "IEditorPanel.h"
#include <vector>
#include <cstddef>

class Renderer;

class EditorUISystem {
public:
    void Register(IEditorPanel* panel, int priority = 0);
    void Unregister(IEditorPanel* panel);

    void ProcessInput();
    void UpdatePanels(float deltaTime);
    void RenderPanels(Renderer& renderer);

    void SetPanelRect(IEditorPanel* panel, float x, float y, float w, float h);
    void SetAllWindowHeight(int h);

    void Clear();

private:
    struct PanelEntry {
        IEditorPanel* panel;
        int priority;
    };

    void Capture(IEditorPanel* panel, MouseEvent::ButtonType button);
    void ReleaseCapture();

    std::vector<PanelEntry> m_panels;
    IEditorPanel* m_capturedPanel = nullptr;
    MouseEvent::ButtonType m_capturedButton = MouseEvent::None;
};
