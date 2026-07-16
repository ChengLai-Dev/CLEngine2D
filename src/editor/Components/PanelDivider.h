#pragma once

#include "IEditorPanel.h"
#include <functional>

class PanelDivider : public IEditorPanel {
public:
    enum class Edge { Left, Right, Horizontal };

    PanelDivider(Edge edge);

    void SetEdgeX(float x);
    void SetDividerHeight(float h);
    void SetDividerTop(float top);

    void SetHorizontalY(float y);
    void SetHorizontalWidth(float w);

    Edge GetEdge() const { return m_edge; }

    // IEditorPanel
    HitRect GetHitRect() const override;
    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;
    bool IsCapturing() const override { return m_isDragging; }

    using ResizeCallback = std::function<void(float pos)>;
    void OnResize(ResizeCallback cb);

    using DragEndCallback = std::function<void()>;
    void OnDragEnd(DragEndCallback cb);

private:
    Edge m_edge;
    float m_edgeX = 0.0f;
    float m_top = 0.0f;
    float m_height = 0.0f;

    float m_horizontalY = 0.0f;
    float m_horizontalWidth = 0.0f;

    bool m_isDragging = false;
    bool m_isHovered = false;
    bool m_hoveredThisFrame = false;

    ResizeCallback m_onResize;
    DragEndCallback m_onDragEnd;

    static constexpr float HANDLE_HALF_WIDTH = 6.0f;
};
