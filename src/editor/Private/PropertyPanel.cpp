#include "PropertyPanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>

PropertyPanel::PropertyPanel() {
    m_rectWidth = 300.0f;
    m_rectHeight = 720.0f;
}

PropertyPanel::~PropertyPanel() = default;

void PropertyPanel::SetTarget(Node* target) {
    m_target = target;
}

Node* PropertyPanel::GetTarget() const {
    return m_target;
}

void PropertyPanel::OnPropertyChanged(PropertyChangedCallback cb) {
    m_onPropertyChanged = std::move(cb);
}

void PropertyPanel::DrawProperty(const char* label, float value, float minVal, float maxVal,
                                  Renderer& renderer, float& y,
                                  std::function<void(float)> setter) {
    (void)minVal;
    (void)maxVal;
    (void)value;
    (void)setter;

    float bgColor[4] = { 0.15f, 0.15f, 0.17f, 1.0f };
    float labelColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

    Mat4 labelBg = Mat4::Translate(Vec3(10.0f + 40.0f, y + 12.0f, 0.0f));
    renderer.DrawQuad(labelBg, Vec2(80.0f, 20.0f), Color(bgColor[0], bgColor[1], bgColor[2], bgColor[3]));

    if (m_fontRenderer && label) {
        float textH = m_fontRenderer->GetLineHeight(1.0f);
        float base = m_fontRenderer->GetBaselineOffset(1.0f);
        m_fontRenderer->RenderString(renderer, label,
            14.0f, y + (24.0f - textH) * 0.5f + base,
            1.0f, labelColor, TextRenderer::Align::Left);
    }

    y += 24.0f;
}

void PropertyPanel::OnRender(Renderer& renderer) {
    Color bgColor(0.12f, 0.12f, 0.14f, 1.0f);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight), bgColor);

    if (m_target) {
        float y = 20.0f;

        Color labelColor(0.9f, 0.6f, 0.2f, 1.0f);

        Vec3 pos = m_target->GetPosition();
        Vec2 size = m_target->GetContentSize();

        if (m_fontRenderer) {
            float titleColor[4] = { 0.9f, 0.6f, 0.2f, 1.0f };
            m_fontRenderer->RenderString(renderer, m_target->GetName().c_str(),
                10.0f, y,
                1.0f, titleColor, TextRenderer::Align::Left);

            char posBuf[64];
            std::snprintf(posBuf, sizeof(posBuf), "Position: %.1f, %.1f", pos.x, pos.y);
            y += 24.0f;
            DrawProperty("Pos X", pos.x, -1000.0f, 1000.0f, renderer, y,
                [this](float v) { m_target->SetPosition(Vec3(v, m_target->GetPosition().y, 0.0f)); });
            DrawProperty("Pos Y", pos.y, -1000.0f, 1000.0f, renderer, y,
                [this](float v) { m_target->SetPosition(Vec3(m_target->GetPosition().x, v, 0.0f)); });

            char sizeBuf[64];
            std::snprintf(sizeBuf, sizeof(sizeBuf), "Size: %.1f x %.1f", size.x, size.y);
            y += 8.0f;
            DrawProperty("Width", size.x, 0.0f, 2000.0f, renderer, y,
                [this](float v) { m_target->SetContentSize(Vec2(v, m_target->GetContentSize().y)); });
            DrawProperty("Height", size.y, 0.0f, 2000.0f, renderer, y,
                [this](float v) { m_target->SetContentSize(Vec2(m_target->GetContentSize().x, v)); });
        }
    }
}
