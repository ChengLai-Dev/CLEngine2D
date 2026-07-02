#include "Gizmo.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>

Gizmo::Gizmo() = default;
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

GizmoHandle::Type Gizmo::HitTestHandle(const Vec3& worldPoint) const {
    if (!m_target) return GizmoHandle::NONE;

    Vec2 size = m_target->GetContentSize();
    const Mat4& world = const_cast<Node*>(m_target)->GetWorldTransform();

    Vec3 corners[4];
    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;
    Vec3 localCorners[4] = {
        Vec3(-hw, -hh, 0.0f),
        Vec3( hw, -hh, 0.0f),
        Vec3( hw,  hh, 0.0f),
        Vec3(-hw,  hh, 0.0f)
    };

    for (int i = 0; i < 4; ++i) {
        corners[i] = world.TransformPoint(localCorners[i]);
    }

    float handleSize = 8.0f;
    Vec3 handlePositions[8] = {
        corners[0],                                      // TOP_LEFT
        Vec3((corners[0].x + corners[1].x) * 0.5f, corners[0].y, 0.0f), // TOP_CENTER
        corners[1],                                      // TOP_RIGHT
        Vec3(corners[0].x, (corners[0].y + corners[3].y) * 0.5f, 0.0f), // MIDDLE_LEFT
        Vec3(corners[1].x, (corners[1].y + corners[2].y) * 0.5f, 0.0f), // MIDDLE_RIGHT
        corners[3],                                      // BOTTOM_LEFT
        Vec3((corners[3].x + corners[2].x) * 0.5f, corners[3].y, 0.0f), // BOTTOM_CENTER
        corners[2]                                       // BOTTOM_RIGHT
    };

    for (int i = 0; i < 8; ++i) {
        float dx = worldPoint.x - handlePositions[i].x;
        float dy = worldPoint.y - handlePositions[i].y;
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
    m_drag.dragOffset = worldStart;
}

void Gizmo::Drag(const Vec3& worldCurrent) {
    if (!m_target || m_drag.handle == GizmoHandle::NONE) return;

    Vec3 delta = worldCurrent - m_drag.dragOffset;

    if (m_mode == GizmoMode::TRANSLATE) {
        m_target->SetPosition(m_drag.startPos + delta);
    } else if (m_mode == GizmoMode::SCALE) {
        Vec2 newSize = m_drag.startSize + Vec2(delta.x, delta.y);
        if (newSize.x < 5.0f) newSize.x = 5.0f;
        if (newSize.y < 5.0f) newSize.y = 5.0f;
        m_target->SetContentSize(newSize);
    }
}

void Gizmo::EndDrag() {
    m_drag = DragState();
}

void Gizmo::Draw(Renderer& renderer, const OrthographicCamera& camera) {
    if (!m_target) return;

    Vec2 size = m_target->GetContentSize();
    const Mat4& world = const_cast<Node*>(m_target)->GetWorldTransform();

    float hw = size.x * 0.5f;
    float hh = size.y * 0.5f;
    Vec3 localCorners[4] = {
        Vec3(-hw, -hh, 0.0f),
        Vec3( hw, -hh, 0.0f),
        Vec3( hw,  hh, 0.0f),
        Vec3(-hw,  hh, 0.0f)
    };

    Vec3 corners[4];
    for (int i = 0; i < 4; ++i) {
        corners[i] = world.TransformPoint(localCorners[i]);
    }

    float selectColor[4] = { 1.0f, 0.5f, 0.0f, 1.0f };

    Vec2 edgeSize(2.0f, 0.0f);

    Vec3 edges[4][2] = {
        { corners[0], corners[1] },
        { corners[1], corners[2] },
        { corners[2], corners[3] },
        { corners[3], corners[0] }
    };

    for (int e = 0; e < 4; ++e) {
        Vec3 mid = Vec3(
            (edges[e][0].x + edges[e][1].x) * 0.5f,
            (edges[e][0].y + edges[e][1].y) * 0.5f,
            0.0f
        );
        float dx = edges[e][1].x - edges[e][0].x;
        float dy = edges[e][1].y - edges[e][0].y;
        float len = std::sqrt(dx * dx + dy * dy);
        if (len > 0.0f) {
            float angle = std::atan2(dy, dx);
            Mat4 edgeTransform =
                Mat4::Translate(mid) *
                Mat4::RotateZ(angle);
            renderer.DrawQuad(edgeTransform, Vec2(len, 3.0f),
                              nullptr, selectColor,
                              0.0f, 0.0f, 1.0f, 1.0f);
        }
    }

    float handleColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    Vec3 handlePositions[8] = {
        corners[0],
        Vec3((corners[0].x + corners[1].x) * 0.5f, corners[0].y, 0.0f),
        corners[1],
        Vec3(corners[0].x, (corners[0].y + corners[3].y) * 0.5f, 0.0f),
        Vec3(corners[1].x, (corners[1].y + corners[2].y) * 0.5f, 0.0f),
        corners[3],
        Vec3((corners[3].x + corners[2].x) * 0.5f, corners[3].y, 0.0f),
        corners[2]
    };

    for (int i = 0; i < 8; ++i) {
        Mat4 handleTransform =
            Mat4::Translate(handlePositions[i]) *
            Mat4::Scale(Vec3(6.0f, 6.0f, 1.0f));
        renderer.DrawQuad(handleTransform, Vec2(6.0f, 6.0f),
                          nullptr, handleColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
    }
}

bool Gizmo::IsDragging() const {
    return m_drag.handle != GizmoHandle::NONE;
}
