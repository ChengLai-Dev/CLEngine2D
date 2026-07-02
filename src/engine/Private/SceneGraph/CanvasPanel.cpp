#include "SceneGraph/CanvasPanel.h"
#include "Render/Renderer.h"

CanvasPanel::CanvasPanel() = default;

CanvasPanel::~CanvasPanel() = default;

void CanvasPanel::AddChildWithAnchor(std::unique_ptr<Widget> child, const FAnchorData& anchor) {
    Widget* raw = child.get();
    m_anchorData[raw] = anchor;
    AddChild(std::move(child));
}

void CanvasPanel::SetChildAnchor(Widget* child, const FAnchorData& anchor) {
    if (child && m_anchorData.find(child) != m_anchorData.end()) {
        m_anchorData[child] = anchor;
    }
}

const FAnchorData* CanvasPanel::GetChildAnchor(Widget* child) const {
    auto it = m_anchorData.find(child);
    return it != m_anchorData.end() ? &it->second : nullptr;
}

void CanvasPanel::UpdateLayout(const Vec2& canvasSize) {
    for (auto& [widget, anchor] : m_anchorData) {
        if (!widget) continue;

        Vec2 finalPos;
        Vec2 finalSize;

        if (anchor.AnchorMin.x == anchor.AnchorMax.x) {
            finalPos.x = canvasSize.x * anchor.AnchorMin.x + anchor.Position.x;
            finalSize.x = anchor.Size.x;
        } else {
            finalPos.x = canvasSize.x * anchor.AnchorMin.x + anchor.Position.x;
            finalSize.x = canvasSize.x * (anchor.AnchorMax.x - anchor.AnchorMin.x) +
                          anchor.Size.x;
        }

        if (anchor.AnchorMin.y == anchor.AnchorMax.y) {
            finalPos.y = canvasSize.y * anchor.AnchorMin.y + anchor.Position.y;
            finalSize.y = anchor.Size.y;
        } else {
            finalPos.y = canvasSize.y * anchor.AnchorMin.y + anchor.Position.y;
            finalSize.y = canvasSize.y * (anchor.AnchorMax.y - anchor.AnchorMin.y) +
                          anchor.Size.y;
        }

        Vec2 pivotOffset(
            finalSize.x * (anchor.Alignment.x - 0.5f),
            finalSize.y * (anchor.Alignment.y - 0.5f)
        );

        widget->SetPosition(Vec3(finalPos.x + pivotOffset.x, finalPos.y + pivotOffset.y, 0.0f));
        widget->SetContentSize(finalSize);
    }
}

void CanvasPanel::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
}
