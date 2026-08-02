#include "EditorUISystem.h"
#include "Input/RawInput.h"
#include "Input/InputCodes.h"
#include <algorithm>

void EditorUISystem::Register(IEditorPanel* panel, int priority) {
    m_panels.push_back({panel, priority});
    std::stable_sort(m_panels.begin(), m_panels.end(),
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

void EditorUISystem::Capture(IEditorPanel* panel, MouseEvent::ButtonType button) {
    m_capturedPanel = panel;
    m_capturedButton = button;
}

void EditorUISystem::ReleaseCapture() {
    m_capturedPanel = nullptr;
    m_capturedButton = MouseEvent::None;
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

    DispatchScroll(pos, scroll);
    DispatchHover(pos);

    if (m_capturedPanel) {
        bool held = (m_capturedButton == MouseEvent::Left) ? leftDown : rightDown;
        bool released = (m_capturedButton == MouseEvent::Left) ? leftReleased : rightReleased;
        DispatchCapture(pos, held, released);
        return;
    }

    if (leftPressed) DispatchPress(pos, MouseEvent::Left);
    if (rightPressed) DispatchPress(pos, MouseEvent::Right);
}

bool EditorUISystem::DispatchScroll(const Vec2& pos, float scrollDelta) {
    if (scrollDelta == 0.0f) return false;
    for (const PanelEntry& entry : m_panels) {
        if (!entry.panel->IsVisible()) continue;
        HitRect hit = entry.panel->GetHitRect();
        if (hit.Contains(pos.x, pos.y)) {
            MouseEvent ev{ MouseEvent::Scroll, pos, MouseEvent::None, scrollDelta };
            if (entry.panel->OnMouseEvent(ev)) return true;
        }
    }
    return false;
}

void EditorUISystem::DispatchHover(const Vec2& pos) {
    for (const PanelEntry& entry : m_panels) {
        if (!entry.panel->IsVisible()) continue;
        HitRect hit = entry.panel->GetHitRect();
        if (hit.Contains(pos.x, pos.y)) {
            MouseEvent ev{ MouseEvent::Move, pos, MouseEvent::None, 0.0f };
            entry.panel->OnMouseEvent(ev);
            return;
        }
    }
}

bool EditorUISystem::DispatchPress(const Vec2& pos, MouseEvent::ButtonType button) {
    for (const PanelEntry& entry : m_panels) {
        if (!entry.panel->IsVisible()) continue;
        HitRect hit = entry.panel->GetHitRect();
        if (hit.Contains(pos.x, pos.y)) {
            MouseEvent ev{ MouseEvent::Press, pos, button, 0.0f };
            if (entry.panel->OnMouseEvent(ev)) {
                if (entry.panel->IsCapturing()) {
                    Capture(entry.panel, button);
                }
                return true;
            }
        }
    }
    return false;
}

void EditorUISystem::DispatchCapture(const Vec2& pos, bool held, bool released) {
    if (released) {
        MouseEvent ev{ MouseEvent::Release, pos, m_capturedButton, 0.0f };
        m_capturedPanel->OnMouseEvent(ev);
        ReleaseCapture();
    } else if (held) {
        MouseEvent ev{ MouseEvent::Move, pos, m_capturedButton, 0.0f };
        m_capturedPanel->OnMouseEvent(ev);
    }
}

void EditorUISystem::UpdatePanels(float deltaTime) {
    for (const PanelEntry& entry : m_panels) {
        entry.panel->OnUpdate(deltaTime);
    }
}

void EditorUISystem::Clear() {
    m_panels.clear();
    ReleaseCapture();
}
