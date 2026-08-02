#include "CanvasView.h"
#include <Cursor.h>
#include <Scene.h>
#include <SceneGraph/UITools.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/Layout.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <TextRenderer.h>
#include <cstdio>

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
    float glviewportY = screenPos.y - m_rectTop;

    float worldX = (glviewportX - halfW) / m_zoomLevel + m_viewCenter.x;
    float worldY = (glviewportY - halfH) / m_zoomLevel + m_viewCenter.y;

    return Vec3(worldX, worldY, 0.0f);
}

Mat4 CanvasView::GetProjection() const {
    float halfW = m_rectWidth * 0.5f / m_zoomLevel;
    float halfH = m_rectHeight * 0.5f / m_zoomLevel;
    return Mat4::Ortho(
        -halfW + m_viewCenter.x,  halfW + m_viewCenter.x,
        halfH + m_viewCenter.y,  -halfH + m_viewCenter.y,
        -1.0f, 1.0f
    );
}

static constexpr float BODY_DRAG_THRESHOLD = 4.0f;
static constexpr GizmoMode kToolbarModes[3] = { GizmoMode::TRANSLATE, GizmoMode::SCALE, GizmoMode::ROTATE };

const CanvasView::DispatchEntry CanvasView::kDispatchTable[8] = {
    { MouseEvent::Press,   MouseEvent::Left,   &CanvasView::OnLeftPress   },
    { MouseEvent::Press,   MouseEvent::Right,  &CanvasView::OnRightPress  },
    { MouseEvent::Move,    MouseEvent::None,   &CanvasView::OnHover       },
    { MouseEvent::Move,    MouseEvent::Left,   &CanvasView::OnLeftHeld    },
    { MouseEvent::Move,    MouseEvent::Right,  &CanvasView::OnRightHeld   },
    { MouseEvent::Release, MouseEvent::Left,   &CanvasView::OnLeftRelease },
    { MouseEvent::Release, MouseEvent::Right,  &CanvasView::OnRightRelease},
    { MouseEvent::Scroll,  MouseEvent::None,   &CanvasView::OnScroll      },
};

bool CanvasView::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    Vec3 worldPos = ScreenToWorld(event.screenPos);
    for (const DispatchEntry& entry : kDispatchTable) {
        if (entry.action == event.type && entry.button == event.button)
            return (this->*entry.handler)(event, worldPos);
    }
    return false;
}

bool CanvasView::OnLeftPress(const MouseEvent& event, const Vec3& worldPos) {
    int tbBtn = HitTestToolbar(event.screenPos);
    if (tbBtn >= 0) {
        m_gizmo->SetMode(kToolbarModes[tbBtn]);
        return true;
    }
    GizmoHandle::Type handle = m_gizmo->HitTestHandle(worldPos);
    if (handle != GizmoHandle::NONE) {
        m_gizmo->BeginDrag(handle, worldPos);
        m_isGizmoDragging = true;
        return true;
    }
    Widget* hit = UITools::HitTestDesign(m_editedScene ? m_editedScene->GetRoot() : nullptr, worldPos);
    if (hit && m_gizmo->GetTarget() && hit == m_gizmo->GetTarget()) {
        m_isBodyDragPending = true;
        m_isBodyDragging = false;
        m_bodyDragStartWorld = worldPos;
        m_bodyDragStartPos = hit->GetPosition();
    } else if (m_onWidgetClicked) {
        m_onWidgetClicked(hit);
    }
    return true;
}

bool CanvasView::OnRightPress(const MouseEvent& event, const Vec3&) {
    m_isPanning = true;
    m_panStartViewCenter = m_viewCenter;
    m_panStartMousePos = event.screenPos;
    return true;
}

bool CanvasView::OnHover(const MouseEvent& event, const Vec3& worldPos) {
    int tbBtn = HitTestToolbar(event.screenPos);
    m_hoveredToolBtn = tbBtn;
    if (tbBtn >= 0) return false;

    GizmoHandle::Type handle = m_gizmo->HitTestHandle(worldPos);
    if (handle != GizmoHandle::NONE) {
        CursorManager::Set(m_gizmo->GetCursorForHandle(handle));
    } else {
        CursorManager::Reset();
    }
    Widget* hit = UITools::HitTestDesign(m_editedScene ? m_editedScene->GetRoot() : nullptr, worldPos);
    m_hoveredWidget = hit;
    return false;
}

bool CanvasView::OnLeftHeld(const MouseEvent&, const Vec3& worldPos) {
    if (m_isGizmoDragging) {
        m_gizmo->Drag(worldPos);
        return true;
    }
    if (m_isBodyDragPending) {
        Vec3 delta = worldPos - m_bodyDragStartWorld;
        float dist = std::sqrt(delta.x * delta.x + delta.y * delta.y);
        if (dist > BODY_DRAG_THRESHOLD) {
            m_isBodyDragging = true;
            m_isBodyDragPending = false;
        }
    }
    if (m_isBodyDragging && m_gizmo && m_gizmo->GetTarget()) {
        Vec3 delta = worldPos - m_bodyDragStartWorld;
        m_gizmo->GetTarget()->SetPosition(m_bodyDragStartPos + delta);
        return true;
    }
    return false;
}

bool CanvasView::OnRightHeld(const MouseEvent& event, const Vec3&) {
    Vec2 totalDelta = event.screenPos - m_panStartMousePos;
    m_viewCenter = m_panStartViewCenter + Vec2(-totalDelta.x, -totalDelta.y) / m_zoomLevel;
    return true;
}

bool CanvasView::OnLeftRelease(const MouseEvent&, const Vec3&) {
    if (m_isGizmoDragging) {
        m_gizmo->EndDrag();
        m_isGizmoDragging = false;
        return true;
    }
    if (m_isBodyDragging || m_isBodyDragPending) {
        m_isBodyDragging = false;
        m_isBodyDragPending = false;
        return true;
    }
    return false;
}

bool CanvasView::OnRightRelease(const MouseEvent&, const Vec3&) {
    m_isPanning = false;
    return true;
}

bool CanvasView::OnScroll(const MouseEvent& event, const Vec3&) {
    Zoom(event.scrollDelta > 0.0f ? 1.1f : 0.9f);
    return true;
}

void CanvasView::OnUpdate(float deltaTime) {
    (void)deltaTime;

    if (!m_isGizmoDragging && !m_isPanning) {
        if (RawInput::IsKeyPressed(KeyCode::W)) {
            m_gizmo->SetMode(GizmoMode::TRANSLATE);
        } else if (RawInput::IsKeyPressed(KeyCode::E)) {
            m_gizmo->SetMode(GizmoMode::SCALE);
        } else if (RawInput::IsKeyPressed(KeyCode::R)) {
            m_gizmo->SetMode(GizmoMode::ROTATE);
        }
    }
}

void CanvasView::DrawGrid(Renderer& renderer) {
    Color gridColor(0.3f, 0.3f, 0.35f, 0.5f);

    float halfW = m_rectWidth * 0.5f / m_zoomLevel;
    float halfH = m_rectHeight * 0.5f / m_zoomLevel;
    float left = -halfW + m_viewCenter.x;
    float right = halfW + m_viewCenter.x;
    float bottom = halfH + m_viewCenter.y;
    float top = -halfH + m_viewCenter.y;

    float spacedGrid = m_gridSize * m_zoomLevel;

    // 确保网格线在屏幕空间的最小间距
    constexpr float MIN_GRID_SPACING = 24.0f;
    while (spacedGrid * m_zoomLevel < MIN_GRID_SPACING) {
        spacedGrid *= 2.0f;
    }

    float startX = std::floor(left / spacedGrid) * spacedGrid;
    float startY = std::floor(bottom / spacedGrid) * spacedGrid;

    for (float x = startX; x <= right; x += spacedGrid) {
        renderer.DrawLine(Vec3(x, bottom, 0.0f), Vec3(x, top, 0.0f), gridColor);
    }

    for (float y = startY; y >= top; y -= spacedGrid) {
        renderer.DrawLine(Vec3(left, y, 0.0f), Vec3(right, y, 0.0f), gridColor);
    }
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
    renderer.Flush();   // 强制网格线先绘制，避免 Line 在 Flush 时叠在 Quad 之上
    m_editedScene->OnRender(renderer);

    // 容器虚线轮廓（未选中、未悬停时显示）
    Node* selectedNode = m_gizmo ? m_gizmo->GetTarget() : nullptr;
    if (m_editedScene->GetRoot()) {
        NodeIterator it(m_editedScene->GetRoot());
        while (Node* node = it.Next()) {
            if (node == selectedNode) continue;
            if (node == m_hoveredWidget) continue;
            if (dynamic_cast<CanvasPanel*>(node) || dynamic_cast<Layout*>(node)) {
                Color dashColor(0.45f, 0.45f, 0.45f, 0.8f);
                float dashThickness = 2.0f / m_zoomLevel;
                DrawDashedWidgetOutline(renderer, node, dashColor, dashThickness);
            }
        }
    }

    float thickness = 3.0f / m_zoomLevel;

    if (m_hoveredWidget && m_hoveredWidget != m_gizmo->GetTarget()) {
        Color hoverColor(0.2f, 0.5f, 1.0f, 1.0f);
        DrawWidgetOutline(renderer, m_hoveredWidget, hoverColor, thickness);
    }

    m_gizmo->SetZoomLevel(m_zoomLevel);
    m_gizmo->Draw(renderer);

    // HUD overlay (screen-space)
    if (m_fontRenderer) {
        renderer.EndScene();

        Mat4 hudProj = Mat4::Ortho(0.0f, m_rectWidth, m_rectHeight, 0.0f, -1.0f, 1.0f);
        renderer.BeginScene(hudProj);

        DrawModeToolbar(renderer);

        float zoomColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        float coordColor[4] = { 0.7f, 0.7f, 0.7f, 1.0f };

        char zoomBuf[32];
        std::snprintf(zoomBuf, sizeof(zoomBuf), "Zoom: %.0f%%", m_zoomLevel * 100.0f);

        Vec2 zoomSize = m_fontRenderer->MeasureString(zoomBuf, 1.0f);
        m_fontRenderer->RenderString(renderer, zoomBuf,
            m_rectWidth - zoomSize.x - 8.0f, m_rectHeight - zoomSize.y - 4.0f,
            1.0f, zoomColor, TextRenderer::Align::Left);

        Vec3 worldPos = ScreenToWorld(mousePos);

        char coordBuf[48];
        std::snprintf(coordBuf, sizeof(coordBuf), "X: %.1f  Y: %.1f", worldPos.x, worldPos.y);
        m_fontRenderer->RenderString(renderer, coordBuf,
            8.0f, m_rectHeight - 20.0f,
            1.0f, coordColor, TextRenderer::Align::Left);
    }

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
        renderer.DrawQuad(mid, Vec3(length, thickness, 1.0f), color, angle);
    }
}

void CanvasView::DrawDashedWidgetOutline(Renderer& renderer, Node* target, const Color& color, float thickness) {
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

    constexpr float DASH_LEN = 6.0f;
    constexpr float GAP_LEN = 4.0f;
    float dashLen = DASH_LEN / m_zoomLevel;
    float gapLen = GAP_LEN / m_zoomLevel;
    float patternLen = dashLen + gapLen;

    for (int e = 0; e < 4; ++e) {
        Vec3 from = edgePairs[e][0];
        Vec3 to = edgePairs[e][1];
        float dx = to.x - from.x;
        float dy = to.y - from.y;
        float edgeLen = std::sqrt(dx * dx + dy * dy);
        if (edgeLen <= 0.0f) continue;

        float angle = std::atan2(dy, dx);
        int numDashes = static_cast<int>((std::ceil)(edgeLen / patternLen));

        for (int d = 0; d < numDashes; ++d) {
            float startOffset = static_cast<float>(d) * patternLen;
            if (startOffset >= edgeLen) break;

            float remaining = edgeLen - startOffset;
            float segLen = dashLen < remaining ? dashLen : remaining;
            float midOffset = startOffset + segLen * 0.5f;

            Vec3 mid(
                from.x + (dx / edgeLen) * midOffset,
                from.y + (dy / edgeLen) * midOffset,
                0.0f
            );
            renderer.DrawQuad(mid, Vec3(segLen, thickness, 1.0f), color, angle);
        }
    }
}

Gizmo* CanvasView::GetGizmo() const {
    return m_gizmo.get();
}

int CanvasView::HitTestToolbar(const Vec2& screenPos) const {
    float lx = screenPos.x - m_rectLeft;
    float ly = screenPos.y - m_rectTop;
    if (ly < 0.0f || ly > TOOLBAR_HEIGHT) return -1;
    int btn = static_cast<int>(lx) / TOOLBAR_BTN_W;
    return (btn >= 0 && btn < 3) ? btn : -1;
}

void CanvasView::DrawModeToolbar(Renderer& renderer) {
    GizmoMode current = m_gizmo->GetMode();
    const char* labels[] = { "Translate", "Scale", "Rotate" };
    GizmoMode modes[] = { GizmoMode::TRANSLATE, GizmoMode::SCALE, GizmoMode::ROTATE };

    for (int i = 0; i < 3; ++i) {
        float bx = static_cast<float>(i * TOOLBAR_BTN_W);
        float by = 0.0f;

        Color bgColor(0.15f, 0.15f, 0.18f, 0.85f);
        if (modes[i] == current) {
            bgColor = Color(0.25f, 0.55f, 0.95f, 0.9f);
        } else if (i == m_hoveredToolBtn) {
            bgColor = Color(0.25f, 0.25f, 0.30f, 0.85f);
        }

        renderer.DrawQuad(Vec3(bx + TOOLBAR_BTN_W * 0.5f, by + TOOLBAR_HEIGHT * 0.5f, 0.0f),
                          Vec3(static_cast<float>(TOOLBAR_BTN_W - 2), static_cast<float>(TOOLBAR_HEIGHT - 2), 1.0f), bgColor);

        if (m_fontRenderer) {
            float labelColor[4] = { 0.9f, 0.9f, 0.9f, 1.0f };
            m_fontRenderer->RenderStringInRect(renderer, labels[i],
                bx, by, TOOLBAR_BTN_W, TOOLBAR_HEIGHT,
                1.0f, labelColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }
    }
}
