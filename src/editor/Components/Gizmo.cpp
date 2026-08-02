#include "Gizmo.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <Render/Texture.h>
#include <vector>
#include <cmath>


Gizmo::Gizmo() {
    static constexpr int S = 16;
    std::vector<unsigned char> pixels(static_cast<size_t>(S) * S * 4, 0);
    int cx = S / 2, cy = S / 2, r = S / 2 - 1;
    for (int y = 0; y < S; ++y) {
        for (int x = 0; x < S; ++x) {
            float dx = static_cast<float>(x - cx);
            float dy = static_cast<float>(y - cy);
            if (dx * dx + dy * dy <= r * r) {
                int idx = (y * S + x) * 4;
                pixels[idx + 0] = 255;
                pixels[idx + 1] = 255;
                pixels[idx + 2] = 255;
                pixels[idx + 3] = 255;
            }
        }
    }
    m_handleDot = std::make_unique<Texture>(S, S, pixels.data());
}

Gizmo::~Gizmo() = default;

void Gizmo::SetTarget(Node* target) {
    m_target = target;
    m_drag = DragState();
}

Node* Gizmo::GetTarget() const {
    return m_target;
}

void Gizmo::SetMode(GizmoMode mode) {
    m_mode = mode;
}

GizmoMode Gizmo::GetMode() const {
    return m_mode;
}

Gizmo::GizmoData Gizmo::ComputeGizmoData() const {
    GizmoData data;

    Vec2 size = m_target->GetContentSize();
    const Mat4& world = const_cast<Node*>(m_target)->GetWorldTransform();

    float halfWidth = size.x * 0.5f;
    float halfHeight = size.y * 0.5f;
    Vec3 localCorners[4] = {
        Vec3(-halfWidth, -halfHeight, 0.0f),
        Vec3( halfWidth, -halfHeight, 0.0f),
        Vec3( halfWidth,  halfHeight, 0.0f),
        Vec3(-halfWidth,  halfHeight, 0.0f)
    };

    for (int i = 0; i < 4; ++i) {
        data.corners[i] = world.TransformPoint(localCorners[i]);
    }

    data.handlePositions[0] = data.corners[0];
    data.handlePositions[1] = Vec3((data.corners[0].x + data.corners[1].x) * 0.5f, data.corners[0].y, 0.0f);
    data.handlePositions[2] = data.corners[1];
    data.handlePositions[3] = Vec3(data.corners[0].x, (data.corners[0].y + data.corners[3].y) * 0.5f, 0.0f);
    data.handlePositions[4] = Vec3(data.corners[1].x, (data.corners[1].y + data.corners[2].y) * 0.5f, 0.0f);
    data.handlePositions[5] = data.corners[3];
    data.handlePositions[6] = Vec3((data.corners[3].x + data.corners[2].x) * 0.5f, data.corners[3].y, 0.0f);
    data.handlePositions[7] = data.corners[2];

    Vec3 edgePairs[4][2] = {
        { data.corners[0], data.corners[1] },
        { data.corners[1], data.corners[2] },
        { data.corners[2], data.corners[3] },
        { data.corners[3], data.corners[0] }
    };
    for (int e = 0; e < 4; ++e) {
        data.edges[e][0] = edgePairs[e][0];
        data.edges[e][1] = edgePairs[e][1];
        float dx = edgePairs[e][1].x - edgePairs[e][0].x;
        float dy = edgePairs[e][1].y - edgePairs[e][0].y;
        data.edgeLengths[e] = std::sqrt(dx * dx + dy * dy);
        data.edgeAngles[e] = std::atan2(dy, dx);
    }

    return data;
}

GizmoHandle::Type Gizmo::HitTestHandle(const Vec3& worldPoint) const {
    if (!m_target) return GizmoHandle::NONE;

    GizmoData data = ComputeGizmoData();

    float handleSize = 8.0f / m_zoomLevel;
    for (int i = 0; i < 8; ++i) {
        float dx = worldPoint.x - data.handlePositions[i].x;
        float dy = worldPoint.y - data.handlePositions[i].y;
        if (dx * dx + dy * dy < handleSize * handleSize) {
            return static_cast<GizmoHandle::Type>(i);
        }
    }

    return GizmoHandle::NONE;
}

void Gizmo::BeginDrag(GizmoHandle::Type handle, const Vec3& worldStart) {
    if (!m_target) return;

    m_drag.handle = handle;
    m_drag.startPos = m_target->GetPosition();
    m_drag.startSize = m_target->GetContentSize();
    m_drag.startScale = m_target->GetScale();
    m_drag.dragStart = worldStart;
}

void Gizmo::Drag(const Vec3& worldCurrent) {
    if (!m_target || m_drag.handle == GizmoHandle::NONE) return;

    Vec3 delta = worldCurrent - m_drag.dragStart;

    if (m_mode == GizmoMode::TRANSLATE) {
        m_target->SetPosition(m_drag.startPos + delta);
    } else if (m_mode == GizmoMode::SCALE) {
        Vec2 anchor = m_target->GetAnchor();
        constexpr float eps = 0.001f;
        if (anchor.x < eps) anchor.x = eps;
        if (anchor.x > 1.0f - eps) anchor.x = 1.0f - eps;
        if (anchor.y < eps) anchor.y = eps;
        if (anchor.y > 1.0f - eps) anchor.y = 1.0f - eps;

        float dx = 1.0f, dy = 1.0f;

        switch (m_drag.handle) {
            case GizmoHandle::TOP_LEFT:
                dx = -1.0f / anchor.x;
                dy = -1.0f / anchor.y;
                break;
            case GizmoHandle::TOP_CENTER:
                dx = 0.0f;
                dy = -1.0f / anchor.y;
                break;
            case GizmoHandle::TOP_RIGHT:
                dx = 1.0f / (1.0f - anchor.x);
                dy = -1.0f / anchor.y;
                break;
            case GizmoHandle::MIDDLE_LEFT:
                dx = -1.0f / anchor.x;
                dy = 0.0f;
                break;
            case GizmoHandle::MIDDLE_RIGHT:
                dx = 1.0f / (1.0f - anchor.x);
                dy = 0.0f;
                break;
            case GizmoHandle::BOTTOM_LEFT:
                dx = -1.0f / anchor.x;
                dy = 1.0f / (1.0f - anchor.y);
                break;
            case GizmoHandle::BOTTOM_CENTER:
                dx = 0.0f;
                dy = 1.0f / (1.0f - anchor.y);
                break;
            case GizmoHandle::BOTTOM_RIGHT:
                dx = 1.0f / (1.0f - anchor.x);
                dy = 1.0f / (1.0f - anchor.y);
                break;
            default: break;
        }

        Vec2 newSize = m_drag.startSize + Vec2(delta.x * dx, delta.y * dy);
        if (newSize.x < 5.0f) newSize.x = 5.0f;
        if (newSize.y < 5.0f) newSize.y = 5.0f;

        m_target->SetContentSize(newSize);
    }
}

void Gizmo::EndDrag() {
    m_drag = DragState();
}

void Gizmo::Draw(Renderer& renderer) {
    if (!m_target) return;

    GizmoData data = ComputeGizmoData();

    Color selectColor(0.2f, 0.8f, 0.2f, 1.0f);
    float edgeThickness = 3.0f / m_zoomLevel;

    for (int e = 0; e < 4; ++e) {
        Vec3 mid = Vec3(
            (data.edges[e][0].x + data.edges[e][1].x) * 0.5f,
            (data.edges[e][0].y + data.edges[e][1].y) * 0.5f,
            0.0f
        );
        if (data.edgeLengths[e] > 0.0f) {
            renderer.DrawQuad(mid, Vec3(data.edgeLengths[e], edgeThickness, 1.0f),
                              selectColor, data.edgeAngles[e]);
        }
    }

    Color white(1.0f, 1.0f, 1.0f, 1.0f);
    float handleScreenSize = 14.0f / m_zoomLevel;
    for (int i = 0; i < 8; ++i) {
        renderer.DrawQuad(data.handlePositions[i],
                          Vec3(handleScreenSize, handleScreenSize, 1.0f),
                          white, 0.0f, m_handleDot.get());
    }
}

void Gizmo::SetZoomLevel(float zoom) {
    m_zoomLevel = zoom;
}

bool Gizmo::IsDragging() const {
    return m_drag.handle != GizmoHandle::NONE;
}

CursorType Gizmo::GetCursorForHandle(GizmoHandle::Type handle) const {
    switch (handle) {
        case GizmoHandle::TOP_LEFT:
        case GizmoHandle::BOTTOM_RIGHT:
            return CursorType::DiagResize2;
        case GizmoHandle::TOP_RIGHT:
        case GizmoHandle::BOTTOM_LEFT:
            return CursorType::DiagResize1;
        case GizmoHandle::TOP_CENTER:
        case GizmoHandle::BOTTOM_CENTER:
            return CursorType::VResize;
        case GizmoHandle::MIDDLE_LEFT:
        case GizmoHandle::MIDDLE_RIGHT:
            return CursorType::HResize;
        default:
            return CursorType::Default;
    }
}
