#pragma once

#include "Math/Vec3.h"

class Widget;
class TextRenderer;

class UISystem {
public:
    static UISystem& GetInstance();

    void SetUIRoot(Widget* root);
    Widget* GetUIRoot() const;

    void SetFontRenderer(TextRenderer* tr) { m_fontRenderer = tr; }
    TextRenderer* GetFontRenderer() const { return m_fontRenderer; }

    void ProcessEvents();

    Widget* GetPressedWidget() const;
    Widget* GetHoveredWidget() const;
    Widget* GetFocusedWidget() const;

    Widget* HitTestScene(const Vec3& worldPoint);

private:
    UISystem() = default;
    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    Widget* HitTestTree(Widget* root, const Vec3& worldPoint);
    void ProcessKeyboardEvents();

    Widget* m_uiRoot = nullptr;
    Widget* m_pressedWidget = nullptr;
    Widget* m_hoveredWidget = nullptr;
    Widget* m_focusedWidget = nullptr;
    TextRenderer* m_fontRenderer = nullptr;
    Vec3 m_lastMousePos;
    bool m_mouseDown = false;
};
