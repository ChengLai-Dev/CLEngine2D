#include "EditorUISystem.h"
#include "Input/RawInput.h"
#include "Input/InputCodes.h"
#include <algorithm>

void EditorUISystem::Register(IEditorPanel* panel, int priority) {
    m_panels.push_back({panel, priority});
    std::sort(m_panels.begin(), m_panels.end(),
        [](const PanelEntry& a, const PanelEntry& b) { return a.priority < b.priority; });
}

void EditorUISystem::Unregister(IEditorPanel* panel) {
    auto it = std::remove_if(m_panels.begin(), m_panels.end(),
        [panel](const PanelEntry& e) { return e.panel == panel; });
    m_panels.erase(it, m_panels.end());
    if (m_capturedPanel == panel) {
        m_capturedPanel = nullptr;
    }
}

void EditorUISystem::Capture(IEditorPanel* panel, int button) {
    m_capturedPanel = panel;
    m_capturedButton = button;
}

void EditorUISystem::ReleaseCapture() {
    m_capturedPanel = nullptr;
    m_capturedButton = -1;
}

void EditorUISystem::ProcessInput() {
    Vec2 pos = RawInput::GetMousePosition();

    bool leftPressed = RawInput::IsMouseButtonPressed(MouseCode::ButtonLeft);
    bool leftDown = RawInput::IsMouseButtonDown(MouseCode::ButtonLeft);
    bool leftReleased = RawInput::IsMouseButtonReleased(MouseCode::ButtonLeft);
    bool rightPressed = RawInput::IsMouseButtonPressed(MouseCode::ButtonRight);
    bool rightDown = RawInput::IsMouseButtonDown(MouseCode::ButtonRight);
    bool rightReleased = RawInput::IsMouseButtonReleased(MouseCode::ButtonRight);
    float scroll = RawInput::GetScrollDeltaY();

    if (scroll != 0.0f) {
        for (const PanelEntry& entry : m_panels) {
            IEditorPanel::HitRect hit = entry.panel->GetHitRect();
            if (hit.Contains(pos.x, pos.y)) {
                MouseEvent ev{ MouseEvent::Scroll, pos, 0, scroll };
                if (entry.panel->OnMouseEvent(ev)) break;
            }
        }
    }

    // Hover: 始终向光标下方面板发送 Move 事件（button=-1 表示非按键移动）
    for (const PanelEntry& entry : m_panels) {
        IEditorPanel::HitRect hit = entry.panel->GetHitRect();
        if (hit.Contains(pos.x, pos.y)) {
            MouseEvent ev{ MouseEvent::Move, pos, -1, 0.0f };
            entry.panel->OnMouseEvent(ev);
            break;
        }
    }

    if (m_capturedPanel) {
        bool held = (m_capturedButton == 0) ? leftDown : rightDown;
        bool released = (m_capturedButton == 0) ? leftReleased : rightReleased;

        if (released) {
            MouseEvent ev{ MouseEvent::Up, pos, m_capturedButton, 0.0f };
            m_capturedPanel->OnMouseEvent(ev);
            ReleaseCapture();
        } else if (held) {
            MouseEvent ev{ MouseEvent::Move, pos, m_capturedButton, 0.0f };
            m_capturedPanel->OnMouseEvent(ev);
        }
        return;
    }

    if (leftPressed) {
        for (const PanelEntry& entry : m_panels) {
            IEditorPanel::HitRect hit = entry.panel->GetHitRect();
            if (hit.Contains(pos.x, pos.y)) {
                MouseEvent ev{ MouseEvent::Down, pos, 0, 0.0f };
                if (entry.panel->OnMouseEvent(ev)) {
                    if (entry.panel->IsCapturing()) {
                        Capture(entry.panel, 0);
                    }
                    break;
                }
            }
        }
    }

    if (rightPressed) {
        for (const PanelEntry& entry : m_panels) {
            IEditorPanel::HitRect hit = entry.panel->GetHitRect();
            if (hit.Contains(pos.x, pos.y)) {
                MouseEvent ev{ MouseEvent::Down, pos, 1, 0.0f };
                if (entry.panel->OnMouseEvent(ev)) {
                    if (entry.panel->IsCapturing()) {
                        Capture(entry.panel, 1);
                    }
                    break;
                }
            }
        }
    }
}

void EditorUISystem::UpdatePanels(float deltaTime) {
    for (const PanelEntry& entry : m_panels) {
        entry.panel->OnUpdate(deltaTime);
    }
}

void EditorUISystem::RenderPanels(Renderer& renderer) {
    for (const PanelEntry& entry : m_panels) {
        entry.panel->OnRender(renderer);
    }
}

void EditorUISystem::SetPanelRect(IEditorPanel* panel, float x, float y, float w, float h) {
    panel->SetRect(x, y, w, h);
}

void EditorUISystem::SetAllWindowHeight(int h) {
    for (const PanelEntry& entry : m_panels) {
        entry.panel->SetWindowHeight(h);
    }
}

void EditorUISystem::Clear() {
    m_panels.clear();
    ReleaseCapture();
}
