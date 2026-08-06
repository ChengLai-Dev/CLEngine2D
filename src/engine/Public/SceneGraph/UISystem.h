#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include <memory>
#include <string>
#include <vector>

class Widget;
class TextRenderer;
class OrthographicCamera;

// UI 画布尺寸（中心原点，与 .cui 根节点坐标模型一致）
constexpr float kUICanvasWidth = 1280.0f;
constexpr float kUICanvasHeight = 720.0f;

class UISystem {
public:
    static UISystem& GetInstance();

    // ---- 多 UI 树叠加层（按 zorder 升序渲染，命中从高到低） ----

    // 解析 .cui 并挂为独立层，返回层容器（Widget，1280x720，不参与自身命中）。
    // 容器生命周期由 UISystem 持有，RemoveUI 摘除后引用即悬垂。
    // modal=true：该层命中失败也阻断下层（模态弹窗不穿透）
    Widget* AddUI(const std::string& filepath, int zorder, bool modal = false);

    // 挂已有 Widget 树为独立层（C++ 侧使用，如 UIDemoScene）
    bool AddLayer(Widget* root, int zorder);

    // 从层列表摘除指定容器/树根；成功返回 true
    bool RemoveUI(Widget* root);

    // 清空全部层（销毁所有 AddUI 创建的容器）。
    // 必须在 Python 解释器存活时调用（层内控件销毁时其回调析构需要 GIL）
    void ClearLayers();

    // 层列表（按 zorder 升序，渲染顺序 = 列表顺序）
    std::vector<Widget*> GetLayers() const;

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

    // 任意 Widget 析构时反向通知：清理指向它的状态指针（回调栈内销毁宿主层的根治防护）。
    // 由 Widget 析构函数调用；不依赖父链遍历，树析构过程中同样安全。
    void OnWidgetDestroyed(Widget* widget);

private:
    // 摘除/销毁层前，清理指向该层内节点的 pressed/hovered/focused 状态指针（防悬垂）
    void ClearWidgetStatesInTree(Widget* root);

    UISystem() = default;
    UISystem(const UISystem&) = delete;
    UISystem& operator=(const UISystem&) = delete;

    Vec3 ScreenToWorld(const Vec2& screenPos) const;

    Widget* HitTestTree(Widget* root, const Vec3& worldPoint);
    void ProcessKeyboardEvents();

    struct UILayer {
        // AddUI 创建的容器由 UISystem 持有所有权；AddLayer 挂外部树时不持有
        std::unique_ptr<Widget> owned;
        Widget* root = nullptr;
        int zorder = 0;
        bool modal = false;
    };
    std::vector<UILayer> m_layers;

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
