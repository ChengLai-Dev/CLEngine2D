#include "CanvasView.h"
#include "Gizmo.h"
#include <Scene.h>
#include <SceneGraph/Node.h>
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>
#include <glad/glad.h>

CanvasView::CanvasView() {
    m_camera = std::make_unique<OrthographicCamera>(0.0f, 100.0f, 100.0f, 0.0f);
    m_gizmo = std::make_unique<Gizmo>();
}

CanvasView::~CanvasView() = default;

void CanvasView::SetEditedScene(Scene* scene) {
    m_editedScene = scene;
}

Scene* CanvasView::GetEditedScene() const {
    return m_editedScene;
}

void CanvasView::SetViewRect(float x, float y, float w, float h) {
    m_viewX = x;
    m_viewY = y;
    m_viewW = w;
    m_viewH = h;
}

void CanvasView::Zoom(float factor) {
    m_zoomLevel *= factor;
    if (m_zoomLevel < 0.1f) m_zoomLevel = 0.1f;
    if (m_zoomLevel > 10.0f) m_zoomLevel = 10.0f;
}

void CanvasView::Pan(const Vec2& delta) {
    m_viewCenter += delta;
}

void CanvasView::ResetView() {
    m_zoomLevel = 1.0f;
    m_viewCenter = Vec2(0.0f, 0.0f);
}

Vec3 CanvasView::ScreenToWorld(const Vec2& screenPos) const {
    float halfW = m_viewW * 0.5f;
    float halfH = m_viewH * 0.5f;

    float worldX = (screenPos.x - m_viewX - halfW) / m_zoomLevel + m_viewCenter.x;
    float worldY = (screenPos.y - m_viewY - halfH) / m_zoomLevel + m_viewCenter.y;

    return Vec3(worldX, worldY, 0.0f);
}

void CanvasView::OnUpdate(float deltaTime) {
    (void)deltaTime;
}

void CanvasView::DrawGrid(Renderer& renderer) {
    float gridColor[4] = { 0.3f, 0.3f, 0.35f, 0.5f };
    float centerColor[4] = { 0.5f, 0.5f, 0.6f, 0.8f };

    float halfW = m_viewW * 0.5f / m_zoomLevel;
    float halfH = m_viewH * 0.5f / m_zoomLevel;
    float left = -halfW + m_viewCenter.x;
    float right = halfW + m_viewCenter.x;
    float bottom = -halfH + m_viewCenter.y;
    float top = halfH + m_viewCenter.y;

    float spacedGrid = m_gridSize * m_zoomLevel;

    float startX = std::floor(left / spacedGrid) * spacedGrid;
    float startY = std::floor(bottom / spacedGrid) * spacedGrid;

    for (float x = startX; x <= right; x += spacedGrid) {
        renderer.DrawLine(Vec3(x, bottom, 0.0f), Vec3(x, top, 0.0f), gridColor);
    }

    for (float y = startY; y <= top; y += spacedGrid) {
        renderer.DrawLine(Vec3(left, y, 0.0f), Vec3(right, y, 0.0f), gridColor);
    }

    float cx = (left + right) * 0.5f;
    float cy = (top + bottom) * 0.5f;
    renderer.DrawLine(Vec3(left, cy, 0.0f), Vec3(right, cy, 0.0f), centerColor);
    renderer.DrawLine(Vec3(cx, bottom, 0.0f), Vec3(cx, top, 0.0f), centerColor);
}

void CanvasView::OnRender(Renderer& renderer) {
    if (!m_editedScene) return;

    float left = -m_viewCenter.x * m_viewW * 0.5f;
    float right = m_viewW * 0.5f / m_zoomLevel + m_viewCenter.x;
    float bottom = m_viewH * 0.5f / m_zoomLevel + m_viewCenter.y;
    float top = -m_viewCenter.y * m_viewH * 0.5f;

    m_camera->SetProjection(
        -m_viewW * 0.5f / m_zoomLevel + m_viewCenter.x,
        m_viewW * 0.5f / m_zoomLevel + m_viewCenter.x,
        -m_viewH * 0.5f / m_zoomLevel + m_viewCenter.y,
        m_viewH * 0.5f / m_zoomLevel + m_viewCenter.y
    );

    RenderCommand::SetViewport(
        static_cast<int>(m_viewX),
        static_cast<int>(m_viewY),
        static_cast<int>(m_viewW),
        static_cast<int>(m_viewH)
    );

    RenderCommand::SetScissor(true);
    RenderCommand::SetScissorRect(
        static_cast<int>(m_viewX),
        static_cast<int>(m_viewY),
        static_cast<int>(m_viewW),
        static_cast<int>(m_viewH)
    );

    renderer.BeginScene(*m_camera);
    DrawGrid(renderer);
    m_editedScene->OnRender(renderer);
    m_gizmo->Draw(renderer, *m_camera);
    renderer.EndScene();

    RenderCommand::SetScissor(false);
}

Gizmo* CanvasView::GetGizmo() const {
    return m_gizmo.get();
}
