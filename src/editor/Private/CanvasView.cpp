#include "CanvasView.h"
#include "Gizmo.h"
#include <Scene.h>
#include <SceneGraph/UISystem.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Input/RawInput.h>

CanvasView::CanvasView() {
    m_rectWidth = 1280.0f;
    m_rectHeight = 720.0f;
    m_gizmo = std::make_unique<Gizmo>();
}

CanvasView::~CanvasView() = default;

void CanvasView::SetEditedScene(Scene* scene) {
    m_editedScene = scene;
}

Scene* CanvasView::GetEditedScene() const {
    return m_editedScene;
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

Mat4 CanvasView::GetProjection() const {
    float halfW = m_rectWidth * 0.5f / m_zoomLevel;
    float halfH = m_rectHeight * 0.5f / m_zoomLevel;
    return Mat4::Ortho(
        -halfW + m_viewCenter.x,  halfW + m_viewCenter.x,
        -halfH + m_viewCenter.y,  halfH + m_viewCenter.y,
        -1.0f, 1.0f
    );
}

bool CanvasView::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    Vec3 worldPos = ScreenToWorld(event.screenPos);

    switch (event.type) {
        case MouseEvent::Down:
            if (event.button == MouseEvent::Left) {
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
            if (event.button == MouseEvent::Right) {
                m_isPanning = true;
                m_panStartViewCenter = m_viewCenter;
                m_panStartMousePos = event.screenPos;
                return true;
            }
            return false;

        case MouseEvent::Move:
            if (event.button == MouseEvent::None) {
                Widget* hit = UISystem::GetInstance().HitTestScene(worldPos);
                m_hoveredWidget = hit;
                return false;
            }
            if (event.button == MouseEvent::Left && m_isGizmoDragging) {
                m_gizmo->Drag(worldPos);
                return true;
            }
            if (event.button == MouseEvent::Right && m_isPanning) {
                Vec2 totalDelta = event.screenPos - m_panStartMousePos;
                m_viewCenter = m_panStartViewCenter + Vec2(-totalDelta.x, totalDelta.y) / m_zoomLevel;
                return true;
            }
            return false;

        case MouseEvent::Up:
            if (event.button == MouseEvent::Left && m_isGizmoDragging) {
                m_gizmo->EndDrag();
                m_isGizmoDragging = false;
                return true;
            }
            if (event.button == MouseEvent::Right && m_isPanning) {
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
    Color gridColor(0.3f, 0.3f, 0.35f, 0.5f);
    Color centerColor(0.5f, 0.5f, 0.6f, 0.8f);

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

    // Scissor: 限制场景内容不溢出 CanvasView 面板边界
    float vpY = static_cast<float>(m_windowHeight) - m_rectTop - m_rectHeight;
    RenderCommand::SetScissor(true);
    RenderCommand::SetScissorRect(
        static_cast<int>(m_rectLeft),
        static_cast<int>(vpY),
        static_cast<int>(m_rectWidth),
        static_cast<int>(m_rectHeight)
    );

    // Hover outline: 鼠标离开时清除残留
    Vec2 mousePos = RawInput::GetMousePosition();
    if (!GetHitRect().Contains(mousePos.x, mousePos.y)) {
        m_hoveredWidget = nullptr;
    }

    DrawGrid(renderer);
    m_editedScene->OnRender(renderer);

    float thickness = 3.0f / m_zoomLevel;

    if (m_hoveredWidget && m_hoveredWidget != m_gizmo->GetTarget()) {
        Color hoverColor(0.2f, 0.5f, 1.0f, 1.0f);
        DrawWidgetOutline(renderer, m_hoveredWidget, hoverColor, thickness);
    }

    m_gizmo->SetZoomLevel(m_zoomLevel);
    m_gizmo->Draw(renderer);

    RenderCommand::SetScissor(false);
}

void CanvasView::DrawWidgetOutline(Renderer& renderer, Node* target, const Color& color, float thickness) {
    if (!target) return;

    Vec2 size = target->GetContentSize();
    const Mat4& world = const_cast<Node*>(target)->GetWorldTransform();

    float halfW = size.x * 0.5f;
    float halfH = size.y * 0.5f;
    Vec3 localCorners[4] = {
        Vec3(-halfW, -halfH, 0.0f),
        Vec3( halfW, -halfH, 0.0f),
        Vec3( halfW,  halfH, 0.0f),
        Vec3(-halfW,  halfH, 0.0f)
    };

    Vec3 corners[4];
    for (int i = 0; i < 4; ++i) {
        corners[i] = world.TransformPoint(localCorners[i]);
    }

    Vec3 edgePairs[4][2] = {
        { corners[0], corners[1] },
        { corners[1], corners[2] },
        { corners[2], corners[3] },
        { corners[3], corners[0] }
    };

    for (int e = 0; e < 4; ++e) {
        Vec3 from = edgePairs[e][0];
        Vec3 to = edgePairs[e][1];
        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float length = std::sqrt(dx * dx + dy * dy);
        if (length <= 0.0f) continue;

        float angle = std::atan2(dy, dx);
        Vec3 mid = Vec3((from.x + to.x) * 0.5f, (from.y + to.y) * 0.5f, 0.0f);
        Mat4 edgeTransform = Mat4::Translate(mid) * Mat4::RotateZ(angle);
        renderer.DrawQuad(edgeTransform, Vec2(length, thickness),
                          color);
    }
}

Gizmo* CanvasView::GetGizmo() const {
    return m_gizmo.get();
}
