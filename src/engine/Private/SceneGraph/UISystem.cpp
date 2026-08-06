#include "SceneGraph/UISystem.h"
#include "SceneGraph/Widget.h"
#include "SceneGraph/Node.h"
#include "UI/UISerializer.h"
#include "Input/InputSystem.h"
#include "Input/RawInput.h"
#include "Render/OrthographicCamera.h"
#include "Math/Mat4.h"
#include "Logger.h"
#include <algorithm>

UISystem& UISystem::GetInstance() {
    static UISystem instance;
    return instance;
}

Widget* UISystem::AddUI(const std::string& filepath, int zorder, bool modal) {
    auto container = std::make_unique<Widget>();
    container->SetName("UIContainer");
    container->SetContentSize(Vec2(kUICanvasWidth, kUICanvasHeight));
    container->SetTouchEnabled(false);

    Node* uiRoot = UISerializer::LoadFromFile(filepath);
    if (!uiRoot) {
        Logger::Error("UISystem::AddUI: failed to load {}", filepath);
        return nullptr;
    }
    container->AddChild(std::unique_ptr<Node>(uiRoot));

    Widget* raw = container.get();
    m_layers.push_back(UILayer{std::move(container), raw, zorder, modal});
    std::stable_sort(m_layers.begin(), m_layers.end(),
                     [](const UILayer& a, const UILayer& b) { return a.zorder < b.zorder; });

    Logger::Info("UISystem::AddUI: loaded {} as layer (zorder={}, modal={}, layer count: {})",
                 filepath, zorder, modal, m_layers.size());
    return raw;
}

bool UISystem::AddLayer(Widget* root, int zorder) {
    if (!root) return false;
    m_layers.push_back(UILayer{nullptr, root, zorder, false});
    std::stable_sort(m_layers.begin(), m_layers.end(),
                     [](const UILayer& a, const UILayer& b) { return a.zorder < b.zorder; });
    return true;
}

bool UISystem::RemoveUI(Widget* root) {
    if (!root) return false;
    for (size_t i = 0; i < m_layers.size(); ++i) {
        if (m_layers[i].root == root) {
            // 清理指向该层内节点的状态指针，避免销毁后悬垂（下一帧 ProcessEvents 崩溃）
            ClearWidgetStatesInTree(root);
            m_layers.erase(m_layers.begin() + static_cast<ptrdiff_t>(i));
            Logger::Info("UISystem::RemoveUI: layer removed (layer count: {})", m_layers.size());
            return true;
        }
    }
    Logger::Warn("UISystem::RemoveUI: layer not found");
    return false;
}

void UISystem::ClearLayers() {
    for (const UILayer& layer : m_layers) {
        ClearWidgetStatesInTree(layer.root);
    }
    m_layers.clear();
    Logger::Info("UISystem::ClearLayers: all layers destroyed");
}

void UISystem::ClearWidgetStatesInTree(Widget* root) {
    auto inTree = [root](Widget* w) {
        for (Node* p = w; p != nullptr; p = p->GetParent()) {
            if (p == root) return true;
        }
        return false;
    };
    if (m_pressedWidget && inTree(m_pressedWidget)) m_pressedWidget = nullptr;
    if (m_hoveredWidget && inTree(m_hoveredWidget)) m_hoveredWidget = nullptr;
    if (m_focusedWidget && inTree(m_focusedWidget)) {
        m_focusedWidget->SetFocused(false);
        m_focusedWidget = nullptr;
    }
}

void UISystem::OnWidgetDestroyed(Widget* widget) {
    if (widget == nullptr) return;
    if (m_pressedWidget == widget) m_pressedWidget = nullptr;
    if (m_hoveredWidget == widget) m_hoveredWidget = nullptr;
    if (m_focusedWidget == widget) {
        // 对象正在析构，不再触碰其成员；仅清状态指针
        m_focusedWidget = nullptr;
    }
    if (m_mouseDown && m_pressedWidget == nullptr) {
        m_mouseDown = false;
    }
}

std::vector<Widget*> UISystem::GetLayers() const {
    std::vector<Widget*> layers;
    layers.reserve(m_layers.size());
    for (const UILayer& layer : m_layers) {
        layers.push_back(layer.root);
    }
    return layers;
}

void UISystem::SetUICamera(const OrthographicCamera* camera) {
    m_uiCamera = camera;
}

void UISystem::SetViewportSize(int width, int height) {
    m_viewportWidth = static_cast<float>(width);
    m_viewportHeight = static_cast<float>(height);
}

Vec3 UISystem::ScreenToWorld(const Vec2& screenPos) const {
    if (m_uiCamera && m_viewportWidth > 0.0f && m_viewportHeight > 0.0f) {
        float ndcX = screenPos.x / m_viewportWidth * 2.0f - 1.0f;
        float ndcY = 1.0f - screenPos.y / m_viewportHeight * 2.0f;
        Mat4 invViewProjection = Mat4::Inverse(m_uiCamera->GetViewProjectionMatrix());
        return invViewProjection.TransformPoint(Vec3(ndcX, ndcY, 0.0f));
    }
    return Vec3(screenPos.x, screenPos.y, 0.0f);
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
    if (m_layers.empty()) return;

    Vec2 pos2d = RawInput::GetMousePosition();
    Vec3 mousePos = ScreenToWorld(pos2d);
    m_lastMousePos = mousePos;

    bool leftDown = RawInput::IsMouseButtonDown(MouseCode::ButtonLeft);
    bool leftPressed = RawInput::IsMouseButtonPressed(MouseCode::ButtonLeft);
    bool leftReleased = RawInput::IsMouseButtonReleased(MouseCode::ButtonLeft);

    // 命中测试：从 zorder 最高层向下遍历，顶层命中即止（模态拦截天然成立）；
    // 模态层测完无命中也阻断（不穿透到下层）
    Widget* hitWidget = nullptr;
    for (size_t i = m_layers.size(); i > 0; --i) {
        const UILayer& layer = m_layers[i - 1];
        hitWidget = HitTestTree(layer.root, mousePos);
        if (hitWidget) break;
        if (layer.modal) break;
    }

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
        // 聚焦控件可能在回调栈内被销毁（OnWidgetDestroyed 已清指针）：判空停止续跑
        if (m_focusedWidget == nullptr) return;
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
    for (size_t i = m_layers.size(); i > 0; --i) {
        const UILayer& layer = m_layers[i - 1];
        Widget* hit = HitTestTree(layer.root, worldPoint);
        if (hit) return hit;
        if (layer.modal) return nullptr;
    }
    return nullptr;
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
