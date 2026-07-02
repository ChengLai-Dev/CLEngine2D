#pragma once

#include "SceneGraph/Widget.h"
#include <unordered_map>

struct FAnchorData {
    Vec2 AnchorMin = Vec2(0.0f, 0.0f);
    Vec2 AnchorMax = Vec2(0.0f, 0.0f);
    Vec2 Alignment = Vec2(0.5f, 0.5f);
    Vec2 Position = Vec2(0.0f, 0.0f);
    Vec2 Size = Vec2(100.0f, 100.0f);
};

class CanvasPanel : public Widget {
public:
    CanvasPanel();
    virtual ~CanvasPanel();

    void AddChildWithAnchor(std::unique_ptr<Widget> child, const FAnchorData& anchor);
    void SetChildAnchor(Widget* child, const FAnchorData& anchor);
    const FAnchorData* GetChildAnchor(Widget* child) const;

    void UpdateLayout(const Vec2& canvasSize);

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    std::unordered_map<Widget*, FAnchorData> m_anchorData;
};
