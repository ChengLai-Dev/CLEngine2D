#pragma once

#include <Cursor.h>
#include <Math/Vec2.h>
#include <Math/Mat4.h>

#include <memory>

class Renderer;
class Node;
class Widget;
class Texture;

enum class GizmoMode { NONE, TRANSLATE, SCALE, ROTATE };

struct GizmoHandle {
    enum Type { TOP_LEFT, TOP_CENTER, TOP_RIGHT, MIDDLE_LEFT,
                MIDDLE_RIGHT, BOTTOM_LEFT, BOTTOM_CENTER, BOTTOM_RIGHT,
                ROTATION, NONE };
};

class Gizmo {
public:
    Gizmo();
    ~Gizmo();

    void SetTarget(Node* target);
    Node* GetTarget() const;

    void SetMode(GizmoMode mode);
    GizmoMode GetMode() const;

    GizmoHandle::Type HitTestHandle(const Vec3& worldPoint) const;
    CursorType GetCursorForHandle(GizmoHandle::Type handle) const;

    void BeginDrag(GizmoHandle::Type handle, const Vec3& worldStart);
    void Drag(const Vec3& worldCurrent);
    void EndDrag();

    void Draw(Renderer& renderer);

    void SetZoomLevel(float zoom);
    float GetZoomLevel() const { return m_zoomLevel; }

    bool IsDragging() const;

private:
    struct GizmoData {
        Vec3 corners[4];
        Vec3 handlePositions[8];
        Vec3 edges[4][2];
        float edgeLengths[4];
        float edgeAngles[4];
    };
    GizmoData ComputeGizmoData() const;

    struct DragState {
        GizmoHandle::Type handle = GizmoHandle::NONE;
        Vec3 startPos;
        Vec2 startSize;
        Vec3 startScale;
        Vec3 dragStart;
    };

    Node* m_target = nullptr;
    GizmoMode m_mode = GizmoMode::SCALE;
    float m_zoomLevel = 1.0f;
    DragState m_drag;
    std::unique_ptr<Texture> m_handleDot = nullptr;
};
