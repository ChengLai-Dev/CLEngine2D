#include "SceneGraph/Label.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"

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

void Label::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    float bgColor[4] = {
        m_color.x,
        m_color.y,
        m_color.z,
        m_color.w * worldOpacity
    };

    renderer.DrawQuad(worldTransform, m_contentSize,
                      m_background.get(), bgColor,
                      0.0f, 0.0f, 1.0f, 1.0f);
}
