#pragma once

#include <Math/Vec2.h>
#include <Math/Vec3.h>
#include <memory>

class Renderer;
class OrthographicCamera;
class Scene;
class Node;
class Gizmo;

class CanvasView {
public:
    CanvasView();
    ~CanvasView();

    void SetEditedScene(Scene* scene);
    Scene* GetEditedScene() const;

    void SetViewRect(float x, float y, float w, float h);

    void Zoom(float factor);
    void Pan(const Vec2& delta);
    void ResetView();

    Vec3 ScreenToWorld(const Vec2& screenPos) const;

    void OnUpdate(float deltaTime);
    void OnRender(Renderer& renderer);

    Gizmo* GetGizmo() const;

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
    Vec2 m_panOffset = Vec2(0.0f, 0.0f);

    float m_gridSize = 50.0f;
};
