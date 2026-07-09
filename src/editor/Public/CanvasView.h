#pragma once

#include "IEditorPanel.h"
#include <Math/Vec2.h>
#include <Math/Vec3.h>
#include <memory>
#include <functional>

class Renderer;
struct Color;
class OrthographicCamera;
class Scene;
class Node;
class Widget;
class Gizmo;

class CanvasView : public IEditorPanel {
public:
    CanvasView();
    ~CanvasView();

    void SetEditedScene(Scene* scene);
    Scene* GetEditedScene() const;

    void SetRect(float x, float y, float w, float h) override;
    void SetWindowHeight(int height) override;
    float GetRectLeft() const { return m_rectLeft; }
    float GetRectTop() const { return m_rectTop; }
    float GetRectWidth() const { return m_rectWidth; }
    float GetRectHeight() const { return m_rectHeight; }

    void Zoom(float factor);
    void Pan(const Vec2& delta);
    void ResetView();

    Vec3 ScreenToWorld(const Vec2& screenPos) const;

    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;

    bool OnMouseEvent(const MouseEvent& event) override;
    HitRect GetHitRect() const override;
    bool IsCapturing() const override { return m_isGizmoDragging || m_isPanning; }

    Gizmo* GetGizmo() const;

    using WidgetClickCallback = std::function<void(Widget*)>;
    void OnWidgetClicked(WidgetClickCallback cb) { m_onWidgetClicked = std::move(cb); }

private:
    void DrawGrid(Renderer& renderer);
    void DrawWidgetOutline(Renderer& renderer, Node* target, const Color& color, float thickness);

    Scene* m_editedScene = nullptr;
    std::unique_ptr<OrthographicCamera> m_camera;
    std::unique_ptr<Gizmo> m_gizmo;

    float m_rectLeft = 0.0f;
    float m_rectTop = 0.0f;
    float m_rectWidth = 1280.0f;
    float m_rectHeight = 720.0f;

    float m_zoomLevel = 1.0f;
    Vec2 m_viewCenter = Vec2(0.0f, 0.0f);

    float m_gridSize = 50.0f;
    int m_windowHeight = 0;

    bool m_isGizmoDragging = false;
    bool m_isPanning = false;
    Vec2 m_panStartViewCenter;
    Vec2 m_panStartMousePos;

    WidgetClickCallback m_onWidgetClicked;

    Widget* m_hoveredWidget = nullptr;
};
