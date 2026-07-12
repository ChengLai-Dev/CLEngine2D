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

    float centerX = indent + (m_rectWidth - indent) * 0.5f;
    float centerY = y + itemHeight * 0.5f;
    Mat4 itemBg = Mat4::Translate(Vec3(centerX, centerY, 0.0f));
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

Node* WidgetTreePanel::HitTest(Node* node, float& y, float my) const {
    if (!node) return nullptr;

    float itemHeight = 22.0f;
    float itemTop = y;
    float itemBottom = y + itemHeight;
    y = itemBottom;

    if (my >= itemTop && my < itemBottom) {
        return node;
    }

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        Node* found = HitTest(node->GetChild(i), y, my);
        if (found) return found;
    }

    return nullptr;
}

bool WidgetTreePanel::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    if (event.type != MouseEvent::Down || event.button != MouseEvent::Left) return false;

    float localY = event.screenPos.y - m_rectTop;
    if (localY < 0.0f || localY >= m_rectHeight) return false;

    if (m_root) {
        float y = 0.0f;
        Node* hit = HitTest(m_root, y, localY);
        if (hit) {
            m_selectedNode = hit;
            if (m_onSelectionChanged) {
                m_onSelectionChanged(hit);
            }
            return true;
        }
    }
    return false;
}

void WidgetTreePanel::SelectNode(Node* node) {
    m_selectedNode = node;
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
