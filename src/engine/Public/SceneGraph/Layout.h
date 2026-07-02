#pragma once

#include "SceneGraph/Widget.h"

class Layout : public Widget {
public:
    enum class Type { VERTICAL, HORIZONTAL, GRID };

    Layout();
    virtual ~Layout();

    void SetLayoutType(Type type);
    Type GetLayoutType() const;

    void SetSpacing(float spacing);
    float GetSpacing() const;

    void SetPadding(const Vec4& padding);
    const Vec4& GetPadding() const;

    void SetGridColumns(int columns);
    int GetGridColumns() const;

    virtual void DoLayout();

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    Type m_layoutType = Type::VERTICAL;
    float m_spacing = 4.0f;
    Vec4 m_padding = Vec4(0.0f, 0.0f, 0.0f, 0.0f);
    int m_gridColumns = 2;
};
