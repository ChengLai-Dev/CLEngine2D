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

    float availW = m_contentSize.x - padLeft - padRight;
    float availH = m_contentSize.y - padTop - padBottom;

    const float fillSize = 100.0f;

    switch (m_layoutType) {
    case Type::VERTICAL: {
        float y = padTop;
        for (size_t i = 0; i < count; ++i) {
            Node* child = GetChild(i);
            float childH = child->GetContentSize().y;
            if (childH <= 0.0f) childH = fillSize;

            child->SetPosition(Vec3(
                padLeft + child->GetContentSize().x * (child->GetAnchor().x - 0.5f),
                y + childH * (child->GetAnchor().y - 0.5f),
                0.0f
            ));

            y += childH + m_spacing;
        }
        break;
    }
    case Type::HORIZONTAL: {
        float x = padLeft;
        for (size_t i = 0; i < count; ++i) {
            Node* child = GetChild(i);
            float childW = child->GetContentSize().x;
            if (childW <= 0.0f) childW = fillSize;

            child->SetPosition(Vec3(
                x + childW * (child->GetAnchor().x - 0.5f),
                padTop + child->GetContentSize().y * (child->GetAnchor().y - 0.5f),
                0.0f
            ));

            x += childW + m_spacing;
        }
        break;
    }
    case Type::GRID: {
        int cols = m_gridColumns;
        if (cols <= 0) cols = 1;
        float cellW = (availW - m_spacing * (cols - 1)) / cols;
        int idx = 0;
        float y = padTop;

        while (idx < static_cast<int>(count)) {
            float x = padLeft;
            for (int c = 0; c < cols && idx < static_cast<int>(count); ++c, ++idx) {
                Node* child = GetChild(idx);
                child->SetPosition(Vec3(
                    x + child->GetContentSize().x * (child->GetAnchor().x - 0.5f),
                    y + child->GetContentSize().y * (child->GetAnchor().y - 0.5f),
                    0.0f
                ));
                x += cellW + m_spacing;
            }
            y += cellW + m_spacing;
        }
        break;
    }
    }
}

void Layout::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
}
