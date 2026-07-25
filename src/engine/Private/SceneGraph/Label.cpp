#include "SceneGraph/Label.h"
#include "SceneGraph/UISystem.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "TextRenderer.h"

Label::Label() = default;

Label::~Label() = default;

void Label::SetText(const std::string& text) {
    m_text = text;
}

const std::string& Label::GetText() const {
    return m_text;
}

void Label::SetFontSize(float size) {
    m_fontSize = size;
}

float Label::GetFontSize() const {
    return m_fontSize;
}

void Label::SetTextColor(const Vec4& color) {
    m_textColor = color;
}

const Vec4& Label::GetTextColor() const {
    return m_textColor;
}

void Label::SetBackground(std::shared_ptr<Texture> texture) {
    m_background = std::move(texture);
}

std::shared_ptr<Texture> Label::GetBackground() const {
    return m_background;
}

void Label::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    Color bgColor(m_color.x, m_color.y, m_color.z, m_color.w * worldOpacity);

    renderer.DrawQuad(worldTransform, m_contentSize,
                    bgColor, m_background.get());

    TextRenderer* tr = UISystem::GetInstance().GetFontRenderer();
    if (!tr || !tr->IsLoaded() || m_text.empty()) return;

    float color[4] = {
        m_textColor.x,
        m_textColor.y,
        m_textColor.z,
        m_textColor.w * worldOpacity
    };

    Vec2 textSize = tr->MeasureString(m_text, 1.0f);
    float scale = m_fontSize / 14.0f;

    float centerX = worldTransform.m[3][0];
    float centerY = worldTransform.m[3][1];
    float textX = centerX - textSize.x * scale * 0.5f;
    float ascent = tr->GetBaselineOffset(scale);
    float textY = centerY + ascent - textSize.y * scale * 0.5f;

    tr->RenderString(renderer, m_text,
        textX, textY,
        scale, color, TextRenderer::Align::Left);
}
