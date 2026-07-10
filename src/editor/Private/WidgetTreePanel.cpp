#include "WidgetTreePanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>

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

void WidgetTreePanel::VisitNode(Node* node, Renderer& renderer, float& y, int depth) {
    if (!node) return;

    float itemHeight = 22.0f;
    float indent = static_cast<float>(depth) * 16.0f;

    Color bgColor(0.15f, 0.15f, 0.17f, 1.0f);
    Color selColor(0.3f, 0.4f, 0.6f, 1.0f);

    const Color& useColor = (node == m_selectedNode) ? selColor : bgColor;

    Mat4 itemBg = Mat4::Translate(Vec3(indent + 2.0f + m_rectWidth * 0.5f, y, 0.0f));
    renderer.DrawQuad(itemBg, Vec2(m_rectWidth - indent - 4.0f, itemHeight),
                      useColor);

    y += itemHeight;

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        VisitNode(node->GetChild(i), renderer, y, depth + 1);
    }
}

void WidgetTreePanel::OnRender(Renderer& renderer) {
    Color bgColor(0.1f, 0.1f, 0.12f, 1.0f);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight));

    if (m_root) {
        float y = 0.0f;
        VisitNode(m_root, renderer, y, 0);
    }
}
