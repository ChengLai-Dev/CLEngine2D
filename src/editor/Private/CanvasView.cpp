#include "CanvasView.h"
#include "Gizmo.h"
#include <Scene.h>
#include <SceneGraph/UISystem.h>
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

void CanvasView::SetRect(float x, float y, float w, float h) {
    m_rectLeft = x;
    m_rectTop = y;
    m_rectWidth = w;
    m_rectHeight = h;
}

void CanvasView::SetWindowHeight(int height) {
    m_windowHeight = height;
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
    float halfW = m_rectWidth * 0.5f;
    float halfH = m_rectHeight * 0.5f;

    float glviewportX = screenPos.x - m_rectLeft;
    float glviewportY = (m_rectTop + m_rectHeight) - screenPos.y;

    float worldX = (glviewportX - halfW) / m_zoomLevel + m_viewCenter.x;
    float worldY = (glviewportY - halfH) / m_zoomLevel + m_viewCenter.y;

    return Vec3(worldX, worldY, 0.0f);
}

IEditorPanel::HitRect CanvasView::GetHitRect() const {
    return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
}

bool CanvasView::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    Vec3 worldPos = ScreenToWorld(event.screenPos);

    switch (event.type) {
        case MouseEvent::Down:
            if (event.button == 0) {
                GizmoHandle::Type handle = m_gizmo->HitTestHandle(worldPos);
                if (handle != GizmoHandle::NONE) {
                    m_gizmo->BeginDrag(handle, worldPos);
                    m_isGizmoDragging = true;
                    return true;
                }
                Widget* hit = UISystem::GetInstance().HitTestScene(worldPos);
                if (m_onWidgetClicked) {
                    m_onWidgetClicked(hit);
                }
                return true;
            }
            if (event.button == 1) {
                m_isPanning = true;
                m_panLastPos = event.screenPos;
                return true;
            }
            return false;

        case MouseEvent::Move:
            if (event.button == 0 && m_isGizmoDragging) {
                m_gizmo->Drag(worldPos);
                return true;
            }
            if (event.button == 1 && m_isPanning) {
                Vec2 delta = event.screenPos - m_panLastPos;
                Pan(Vec2(-delta.x, delta.y) * 0.5f);
                m_panLastPos = event.screenPos;
                return true;
            }
            return false;

        case MouseEvent::Up:
            if (event.button == 0 && m_isGizmoDragging) {
                m_gizmo->EndDrag();
                m_isGizmoDragging = false;
                return true;
            }
            if (event.button == 1 && m_isPanning) {
                m_isPanning = false;
                return true;
            }
            return false;

        case MouseEvent::Scroll:
            Zoom(event.scrollDelta > 0.0f ? 1.1f : 0.9f);
            return true;
    }
    return false;
}

void CanvasView::OnUpdate(float deltaTime) {
    (void)deltaTime;
}

void CanvasView::DrawGrid(Renderer& renderer) {
    float gridColor[4] = { 0.3f, 0.3f, 0.35f, 0.5f };
    float centerColor[4] = { 0.5f, 0.5f, 0.6f, 0.8f };

    float halfW = m_rectWidth * 0.5f / m_zoomLevel;
    float halfH = m_rectHeight * 0.5f / m_zoomLevel;
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

    m_camera->SetProjection(
        -m_rectWidth * 0.5f / m_zoomLevel + m_viewCenter.x,
        m_rectWidth * 0.5f / m_zoomLevel + m_viewCenter.x,
        -m_rectHeight * 0.5f / m_zoomLevel + m_viewCenter.y,
        m_rectHeight * 0.5f / m_zoomLevel + m_viewCenter.y
    );

    float vpY = static_cast<float>(m_windowHeight) - m_rectTop - m_rectHeight;
    RenderCommand::SetViewport(
        static_cast<int>(m_rectLeft),
        static_cast<int>(vpY),
        static_cast<int>(m_rectWidth),
        static_cast<int>(m_rectHeight)
    );

    RenderCommand::SetScissor(true);
    RenderCommand::SetScissorRect(
        static_cast<int>(m_rectLeft),
        static_cast<int>(vpY),
        static_cast<int>(m_rectWidth),
        static_cast<int>(m_rectHeight)
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
