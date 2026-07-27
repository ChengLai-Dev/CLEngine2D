#pragma once

#include "IEditorPanel.h"
#include "PopupManager.h"
#include <vector>
#include <cstddef>

class EditorUISystem {
public:
    void Register(IEditorPanel* panel, int priority = 0);
    void Unregister(IEditorPanel* panel);

    void ProcessInput();
    void UpdatePanels(float deltaTime);

    void OpenPopup(float screenX, float screenY,
                   const PopupMenu::Item* items, int count,
                   std::function<void(int)> onSelected = nullptr,
                   std::function<void()> onDismissed = nullptr);
    void ClosePopup();
    bool IsPopupOpen() const;
    void DrawPopup(Renderer& renderer, TextRenderer* font) const;

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
    PopupManager m_popup;
};
