#include "SceneGraph/Layout.h"

Layout::Layout() = default;

Layout::~Layout() = default;

void Layout::SetLayoutType(Type type) {
    m_layoutType = type;
}

Layout::Type Layout::GetLayoutType() const {
    return m_layoutType;
}

void Layout::SetSpacing(float spacing) {
    m_spacing = spacing;
}

float Layout::GetSpacing() const {
    return m_spacing;
}

void Layout::SetPadding(const Vec4& padding) {
    m_padding = padding;
}

const Vec4& Layout::GetPadding() const {
    return m_padding;
}

void Layout::SetGridColumns(int columns) {
    m_gridColumns = columns > 0 ? columns : 1;
}

int Layout::GetGridColumns() const {
    return m_gridColumns;
}

void Layout::DoLayout() {
    size_t count = GetChildCount();
    if (count == 0) return;

    float padLeft = m_padding.x;
    float padTop = m_padding.y;
    float padRight = m_padding.z;
    float padBottom = m_padding.w;

    // 内容区边界（局部坐标，锚点语义：position = 锚点位置）
    float contentLeft = -m_anchor.x * m_contentSize.x + padLeft;
    float contentTop = (1.0f - m_anchor.y) * m_contentSize.y - padTop;
    float contentBottom = -m_anchor.y * m_contentSize.y + padBottom;

    float availW = m_contentSize.x - padLeft - padRight;

    const float fillSize = 100.0f;

    switch (m_layoutType) {
    case Type::VERTICAL: {
        float itemTop = contentTop;
        for (size_t i = 0; i < count; ++i) {
            Node* child = GetChild(i);
            float childW = child->GetContentSize().x;
            float childH = child->GetContentSize().y;
            if (childH <= 0.0f) childH = fillSize;

            child->SetPosition(Vec3(
                contentLeft + childW * child->GetAnchor().x,
                itemTop + childH * (child->GetAnchor().y - 1.0f),
                0.0f
            ));

            itemTop -= childH + m_spacing;
        }
        break;
    }
    case Type::HORIZONTAL: {
        float centerY = (contentTop + contentBottom) * 0.5f;
        float itemLeft = contentLeft;
        for (size_t i = 0; i < count; ++i) {
            Node* child = GetChild(i);
            float childW = child->GetContentSize().x;
            float childH = child->GetContentSize().y;
            if (childW <= 0.0f) childW = fillSize;

            child->SetPosition(Vec3(
                itemLeft + childW * child->GetAnchor().x,
                centerY + childH * (child->GetAnchor().y - 0.5f),
                0.0f
            ));

            itemLeft += childW + m_spacing;
        }
        break;
    }
    case Type::GRID: {
        int cols = m_gridColumns;
        if (cols <= 0) cols = 1;
        float cellW = (availW - m_spacing * (cols - 1)) / cols;
        int idx = 0;
        float itemTop = contentTop;

        while (idx < static_cast<int>(count)) {
            float itemLeft = contentLeft;
            for (int c = 0; c < cols && idx < static_cast<int>(count); ++c, ++idx) {
                Node* child = GetChild(idx);
                float childW = child->GetContentSize().x;
                float childH = child->GetContentSize().y;

                child->SetPosition(Vec3(
                    itemLeft + childW * child->GetAnchor().x,
                    itemTop + childH * (child->GetAnchor().y - 1.0f),
                    0.0f
                ));
                itemLeft += cellW + m_spacing;
            }
            itemTop -= cellW + m_spacing;
        }
        break;
    }
    }
}

void Layout::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
}
