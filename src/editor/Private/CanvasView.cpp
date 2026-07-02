#include "CanvasView.h"
#include "Gizmo.h"
#include <Scene.h>
#include <SceneGraph/Node.h>
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>
#include <glad/glad.h>

CanvasView::CanvasView() {
    m_camera = std::make_unique<OrthographicCamera>(0.0f, 1280.0f, 720.0f, 0.0f);
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
    m_panOffset += delta;
}

void CanvasView::ResetView() {
    m_zoomLevel = 1.0f;
    m_panOffset = Vec2(0.0f, 0.0f);
}

Vec3 CanvasView::ScreenToWorld(const Vec2& screenPos) const {
    float halfW = m_viewW * 0.5f;
    float halfH = m_viewH * 0.5f;

    float worldX = (screenPos.x - m_viewX - halfW) / m_zoomLevel + m_panOffset.x;
    float worldY = (screenPos.y - m_viewY - halfH) / m_zoomLevel + m_panOffset.y;

    return Vec3(worldX, worldY, 0.0f);
}

void CanvasView::OnUpdate(float deltaTime) {
    (void)deltaTime;
}

void CanvasView::DrawGrid(Renderer& renderer) {
    float gridColor[4] = { 0.3f, 0.3f, 0.35f, 0.5f };
    float centerColor[4] = { 0.5f, 0.5f, 0.6f, 0.8f };

    float left = -m_panOffset.x * m_zoomLevel - m_viewW * 0.5f;
    float right = -m_panOffset.x * m_zoomLevel + m_viewW * 0.5f;
    float bottom = -m_panOffset.y * m_zoomLevel - m_viewH * 0.5f;
    float top = -m_panOffset.y * m_zoomLevel + m_viewH * 0.5f;

    float spacedGrid = m_gridSize * m_zoomLevel;

    float startX = std::floor(left / spacedGrid) * spacedGrid;
    float startY = std::floor(bottom / spacedGrid) * spacedGrid;

    for (float x = startX; x <= right; x += spacedGrid) {
        float len = top - bottom;
        Vec3 start(x, bottom, 0.0f);
        Vec3 end(x, top, 0.0f);
        Mat4 transform = Mat4::Translate(Vec3((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f, 0.0f));
        renderer.DrawQuad(transform, Vec2(1.0f, len),
                          nullptr, gridColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
    }

    for (float y = startY; y <= top; y += spacedGrid) {
        float len = right - left;
        Vec3 start(left, y, 0.0f);
        Vec3 end(right, y, 0.0f);
        Mat4 transform = Mat4::Translate(Vec3((start.x + end.x) * 0.5f, (start.y + end.y) * 0.5f, 0.0f));
        renderer.DrawQuad(transform, Vec2(len, 1.0f),
                          nullptr, gridColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
    }

    {
        Mat4 cx = Mat4::Translate(Vec3(0.0f, (top + bottom) * 0.5f, 0.0f));
        renderer.DrawQuad(cx, Vec2(right - left, 2.0f),
                          nullptr, centerColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
    }
    {
        Mat4 cy = Mat4::Translate(Vec3((left + right) * 0.5f, 0.0f, 0.0f));
        renderer.DrawQuad(cy, Vec2(2.0f, top - bottom),
                          nullptr, centerColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
    }
}

void CanvasView::OnRender(Renderer& renderer) {
    if (!m_editedScene) return;

    float left = -m_panOffset.x * m_viewW * 0.5f;
    float right = m_viewW * 0.5f / m_zoomLevel + m_panOffset.x;
    float bottom = m_viewH * 0.5f / m_zoomLevel + m_panOffset.y;
    float top = -m_panOffset.y * m_viewH * 0.5f;

    m_camera->SetProjection(
        -m_viewW * 0.5f / m_zoomLevel + m_panOffset.x,
        m_viewW * 0.5f / m_zoomLevel + m_panOffset.x,
        -m_viewH * 0.5f / m_zoomLevel + m_panOffset.y,
        m_viewH * 0.5f / m_zoomLevel + m_panOffset.y
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
    RenderCommand::SetViewport(0, 0, 1280, 720);
}

Gizmo* CanvasView::GetGizmo() const {
    return m_gizmo.get();
}
