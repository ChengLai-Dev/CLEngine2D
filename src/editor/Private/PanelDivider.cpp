#include "PanelDivider.h"
#include <Render/Renderer.h>
#include <Cursor.h>

PanelDivider::PanelDivider(Edge edge)
    : m_edge(edge)
{
}

void PanelDivider::SetEdgeX(float x) {
    m_edgeX = x;
}

void PanelDivider::SetDividerHeight(float h) {
    m_height = h;
}

void PanelDivider::SetDividerTop(float top) {
    m_top = top;
}

HitRect PanelDivider::GetHitRect() const {
    return {
        m_edgeX - HANDLE_HALF_WIDTH,
        m_top,
        HANDLE_HALF_WIDTH * 2.0f,
        m_height
    };
}

void PanelDivider::OnUpdate(float) {
    if (!m_isDragging) {
        m_isHovered = m_hoveredThisFrame;
    }
    m_hoveredThisFrame = false;
}

void PanelDivider::OnRender(Renderer& renderer) {
    if (!m_isHovered && !m_isDragging) return;

    Color highlight(0.7f, 0.8f, 1.0f, 1.0f);

    float cx = HANDLE_HALF_WIDTH;
    float cy = m_height * 0.5f;
    Mat4 xform = Mat4::Translate(Vec3(cx, cy, 0.0f));
    renderer.DrawQuad(xform, Vec2(2.0f, m_height), highlight);
}

bool PanelDivider::OnMouseEvent(const MouseEvent& event) {
    float localY = event.screenPos.y - m_top;

    switch (event.type) {
        case MouseEvent::Move: {
            if (event.button == MouseEvent::None) {
                bool inRange = localY >= 0.0f && localY < m_height;
                if (inRange) {
                    CursorManager::Set(CursorType::HResize);
                }
                m_hoveredThisFrame = inRange;
                return true;
            }

            if (m_isDragging) {
                if (m_onResize) {
                    m_onResize(event.screenPos.x);
                }
                return true;
            }
            return false;
        }

        case MouseEvent::Press: {
            if (event.button != MouseEvent::Left) return false;
            if (localY < 0.0f || localY >= m_height) return false;

            m_isDragging = true;
            m_hoveredThisFrame = true;
            return true;
        }

        case MouseEvent::Release: {
            if (event.button != MouseEvent::Left) return false;
            if (!m_isDragging) return false;

            m_isDragging = false;
            m_isHovered = false;
            m_hoveredThisFrame = false;
            CursorManager::Reset();
            if (m_onDragEnd) {
                m_onDragEnd();
            }
            return true;
        }

        default:
            return false;
    }
}

void PanelDivider::OnResize(ResizeCallback cb) {
    m_onResize = std::move(cb);
}

void PanelDivider::OnDragEnd(DragEndCallback cb) {
    m_onDragEnd = std::move(cb);
}
