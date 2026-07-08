#pragma once

#include "IEditorPanel.h"
#include <Math/Vec2.h>
#include <Math/Vec3.h>
#include <memory>
#include <functional>

class Renderer;
class OrthographicCamera;
class Scene;
class Node;
class Gizmo;

class CanvasView : public IEditorPanel {
public:
    CanvasView();
    ~CanvasView();

    void SetEditedScene(Scene* scene);
    Scene* GetEditedScene() const;

    void SetRect(float x, float y, float w, float h) override;
    void SetWindowHeight(int height) override;
    float GetViewX() const { return m_viewX; }
    float GetViewY() const { return m_viewY; }
    float GetViewW() const { return m_viewW; }
    float GetViewH() const { return m_viewH; }

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

    using CanvasClickCallback = std::function<void(const Vec3& worldPos)>;
    void OnCanvasClick(CanvasClickCallback cb) { m_onCanvasClick = std::move(cb); }

private:
    void DrawGrid(Renderer& renderer);

    Scene* m_editedScene = nullptr;
    std::unique_ptr<OrthographicCamera> m_camera;
    std::unique_ptr<Gizmo> m_gizmo;

    float m_viewX = 0.0f;
    float m_viewY = 0.0f;
    float m_viewW = 1280.0f;
    float m_viewH = 720.0f;

    float m_zoomLevel = 1.0f;
    Vec2 m_viewCenter = Vec2(0.0f, 0.0f);

    float m_gridSize = 50.0f;
    int m_windowHeight = 0;

    bool m_isGizmoDragging = false;
    bool m_isPanning = false;
    Vec2 m_panLastPos;

    CanvasClickCallback m_onCanvasClick;
};
