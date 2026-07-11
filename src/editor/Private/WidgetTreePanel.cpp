#include "WidgetTreePanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>

WidgetTreePanel::WidgetTreePanel() {
    m_rectWidth = 250.0f;
    m_rectHeight = 720.0f;
}

WidgetTreePanel::~WidgetTreePanel() = default;

void WidgetTreePanel::SetRoot(Node* root) {
    m_root = root;
}

void WidgetTreePanel::OnSelectionChanged(SelectionChangedCallback cb) {
    m_onSelectionChanged = std::move(cb);
}

Node* WidgetTreePanel::GetSelectedNode() const {
    return m_selectedNode;
}

void WidgetTreePanel::DrawWidgetTree(Node* node, Renderer& renderer, float& y, int depth) {
    if (!node) return;

    float itemHeight = 22.0f;
    float indent = static_cast<float>(depth) * 16.0f;

    Color bgColor(0.15f, 0.15f, 0.17f, 1.0f);
    Color selColor(0.3f, 0.4f, 0.6f, 1.0f);

    const Color& useColor = (node == m_selectedNode) ? selColor : bgColor;

    Mat4 itemBg = Mat4::Translate(Vec3(indent + 2.0f + m_rectWidth * 0.5f, y, 0.0f));
    renderer.DrawQuad(itemBg, Vec2(m_rectWidth - indent - 4.0f, itemHeight),
                      useColor);

    if (m_fontRenderer && !node->GetName().empty()) {
        float textColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        float textH = m_fontRenderer->GetLineHeight(1.0f);
        float base = m_fontRenderer->GetBaselineOffset(1.0f);
        m_fontRenderer->RenderString(renderer, node->GetName(),
            indent + 6.0f, y + (itemHeight - textH) * 0.5f + base,
            1.0f, textColor, TextRenderer::Align::Left);
    }

    y += itemHeight;

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        DrawWidgetTree(node->GetChild(i), renderer, y, depth + 1);
    }
}

void WidgetTreePanel::OnRender(Renderer& renderer) {
    Color bgColor(0.1f, 0.1f, 0.12f, 1.0f);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight));

    if (m_root) {
        float y = 0.0f;
        DrawWidgetTree(m_root, renderer, y, 0);
    }
}
