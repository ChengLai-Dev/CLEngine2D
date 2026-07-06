#include "WidgetTreePanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>

WidgetTreePanel::WidgetTreePanel() {
    m_camera = std::make_unique<OrthographicCamera>(0.0f, 100.0f, 100.0f, 0.0f);
}

WidgetTreePanel::~WidgetTreePanel() = default;

void WidgetTreePanel::SetRoot(Node* root) {
    m_root = root;
}

void WidgetTreePanel::SetRect(float x, float y, float w, float h) {
    m_rectX = x;
    m_rectY = y;
    m_rectW = w;
    m_rectH = h;
    m_camera->SetProjection(0.0f, w, h, 0.0f);
}

void WidgetTreePanel::OnSelectionChanged(SelectionChangedCallback cb) {
    m_onSelectionChanged = std::move(cb);
}

Node* WidgetTreePanel::GetSelectedNode() const {
    return m_selectedNode;
}

void WidgetTreePanel::VisitNode(Node* node, Renderer& renderer, float& y, int depth) {
    if (!node) return;

    float itemH = 22.0f;
    float indent = static_cast<float>(depth) * 16.0f;

    float bgColor[4] = { 0.15f, 0.15f, 0.17f, 1.0f };
    float textColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
    float selColor[4] = { 0.3f, 0.4f, 0.6f, 1.0f };

    float* useColor = (node == m_selectedNode) ? selColor : bgColor;

    Mat4 itemBg = Mat4::Translate(Vec3(indent + 2.0f + m_rectW * 0.5f, y, 0.0f));
    renderer.DrawQuad(itemBg, Vec2(m_rectW - indent - 4.0f, itemH),
                      nullptr, useColor,
                      0.0f, 0.0f, 1.0f, 1.0f);

    y += itemH;

    for (size_t i = 0; i < node->GetChildCount(); ++i) {
        VisitNode(node->GetChild(i), renderer, y, depth + 1);
    }
}

void WidgetTreePanel::OnRender(Renderer& renderer) {
    float bgColor[4] = { 0.1f, 0.1f, 0.12f, 1.0f };

    RenderCommand::SetViewport(
        static_cast<int>(m_rectX),
        static_cast<int>(m_rectY),
        static_cast<int>(m_rectW),
        static_cast<int>(m_rectH)
    );

    renderer.BeginScene(*m_camera);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectW * 0.5f, m_rectH * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectW, m_rectH),
                      nullptr, bgColor,
                      0.0f, 0.0f, 1.0f, 1.0f);

    if (m_root) {
        float y = 0.0f;
        VisitNode(m_root, renderer, y, 0);
    }

    renderer.EndScene();
}
