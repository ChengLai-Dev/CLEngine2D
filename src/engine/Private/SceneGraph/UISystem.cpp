#include "SceneGraph/UISystem.h"
#include "SceneGraph/Widget.h"
#include "Input/InputSystem.h"
#include "Input/RawInput.h"

UISystem& UISystem::GetInstance() {
    static UISystem instance;
    return instance;
}

void UISystem::SetUIRoot(Widget* root) {
    m_uiRoot = root;
}

Widget* UISystem::GetUIRoot() const {
    return m_uiRoot;
}

Widget* UISystem::GetPressedWidget() const {
    return m_pressedWidget;
}

Widget* UISystem::GetHoveredWidget() const {
    return m_hoveredWidget;
}

Widget* UISystem::GetFocusedWidget() const {
    return m_focusedWidget;
}

void UISystem::ProcessEvents() {
    if (!m_uiRoot) return;

    Vec2 pos2d = RawInput::GetMousePosition();
    Vec3 mousePos(pos2d.x, pos2d.y, 0.0f);
    m_lastMousePos = mousePos;

    bool leftDown = RawInput::IsMouseButtonDown(MouseCode::ButtonLeft);
    bool leftPressed = RawInput::IsMouseButtonPressed(MouseCode::ButtonLeft);
    bool leftReleased = RawInput::IsMouseButtonReleased(MouseCode::ButtonLeft);

    Widget* hitWidget = HitTestTree(m_uiRoot, mousePos);

    if (hitWidget != m_hoveredWidget) {
        m_hoveredWidget = hitWidget;
    }

    if (leftPressed) {
        Widget* target = hitWidget;
        if (target && target->IsTouchEnabled() && target->IsEnabled()) {
            m_pressedWidget = target;
            m_mouseDown = true;

            if (target->IsFocusable()) {
                if (m_focusedWidget && m_focusedWidget != target) {
                    m_focusedWidget->SetFocused(false);
                }
                target->SetFocused(true);
                m_focusedWidget = target;
            }

            target->OnTouchStartedEvent(Vec2(mousePos.x, mousePos.y));
        }
    } else if (leftReleased && m_mouseDown) {
        m_mouseDown = false;

        if (m_pressedWidget) {
            Widget* releaseTarget = hitWidget;
            if (releaseTarget == m_pressedWidget) {
                m_pressedWidget->OnTouchEndedEvent(Vec2(mousePos.x, mousePos.y));
            }
            m_pressedWidget = nullptr;
        }
    } else if (m_mouseDown && m_pressedWidget) {
        m_pressedWidget->OnTouchMovedEvent(Vec2(mousePos.x, mousePos.y));
    }

    ProcessKeyboardEvents();
}

void UISystem::ProcessKeyboardEvents() {
    if (!m_focusedWidget) return;

    for (uint16_t code = 0; code < 349; ++code) {
        KeyCode key = static_cast<KeyCode>(code);

        if (RawInput::IsKeyPressed(key)) {
            if (m_focusedWidget->OnKeyDownEvent(key)) {
                InputSystem::GetInstance().MarkKeyConsumedByUI(code, false);
            }
        }

        if (RawInput::IsKeyReleased(key)) {
            if (m_focusedWidget->OnKeyUpEvent(key)) {
                InputSystem::GetInstance().MarkKeyConsumedByUI(code, false);
            }
        }
    }
}

Widget* UISystem::HitTestScene(const Vec3& worldPoint) {
    if (!m_uiRoot) return nullptr;
    return HitTestTree(m_uiRoot, worldPoint);
}

Widget* UISystem::HitTestTree(Widget* root, const Vec3& worldPoint) {
    if (!root || !root->IsVisible() || !root->IsEnabled()) return nullptr;

    Widget* result = nullptr;

    for (size_t i = root->GetChildCount(); i > 0; --i) {
        Widget* child = dynamic_cast<Widget*>(root->GetChild(i - 1));
        if (child) {
            Widget* found = HitTestTree(child, worldPoint);
            if (found) return found;
        }
    }

    if (root->IsTouchEnabled() && root->HitTest(worldPoint)) {
        return root;
    }

    return nullptr;
}
