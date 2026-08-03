#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"

class Widget;
class TextRenderer;
class OrthographicCamera;

class UISystem {
public:
    static UISystem& GetInstance();

    void SetUIRoot(Widget* root);
    Widget* GetUIRoot() const;

    void SetFontRenderer(TextRenderer* tr) { m_fontRenderer = tr; }
    TextRenderer* GetFontRenderer() const { return m_fontRenderer; }

    // UI 相机与视口尺寸由应用层注入，命中测试通过逆 ViewProjection 做坐标转换
    void SetUICamera(const OrthographicCamera* camera);
    void SetViewportSize(int width, int height);

    void ProcessEvents();

    Widget* GetPressedWidget() const;
    Widget* GetHoveredWidget() const;
    Widget* GetFocusedWidget() const;

    Widget* HitTestScene(const Vec3& worldPoint);

private:
    UISystem() = default;
    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    Vec3 ScreenToWorld(const Vec2& screenPos) const;

    Widget* HitTestTree(Widget* root, const Vec3& worldPoint);
    void ProcessKeyboardEvents();

    Widget* m_uiRoot = nullptr;
    Widget* m_pressedWidget = nullptr;
    Widget* m_hoveredWidget = nullptr;
    Widget* m_focusedWidget = nullptr;
    TextRenderer* m_fontRenderer = nullptr;
    const OrthographicCamera* m_uiCamera = nullptr;
    float m_viewportWidth = 0.0f;
    float m_viewportHeight = 0.0f;
    Vec3 m_lastMousePos;
    bool m_mouseDown = false;
};
