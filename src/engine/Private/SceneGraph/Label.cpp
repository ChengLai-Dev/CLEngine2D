#include "SceneGraph/Label.h"
#include "SceneGraph/UISystem.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"
#include "TextRenderer.h"
#include <vector>

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

void Label::SetHAlign(Align align) {
    m_hAlign = align;
}

Label::Align Label::GetHAlign() const {
    return m_hAlign;
}

void Label::SetVAlign(VAlign align) {
    m_vAlign = align;
}

Label::VAlign Label::GetVAlign() const {
    return m_vAlign;
}

void Label::SetLineSpacing(float spacing) {
    m_lineSpacing = spacing > 0.0f ? spacing : 1.0f;
}

float Label::GetLineSpacing() const {
    return m_lineSpacing;
}

void Label::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    // 无背景纹理时不画背景 quad（Label 默认透明；需要底色时用 SetBackground 提供纹理）
    if (m_background) {
        Color bgColor(m_color.x, m_color.y, m_color.z, m_color.w * worldOpacity);
        renderer.DrawQuad(worldTransform, m_contentSize, bgColor, m_background.get());
    }

    TextRenderer* tr = UISystem::GetInstance().GetFontRenderer();
    if (!tr || !tr->IsLoaded() || m_text.empty()) return;

    float color[4] = {
        m_textColor.x,
        m_textColor.y,
        m_textColor.z,
        m_textColor.w * worldOpacity
    };

    float scale = m_fontSize / 14.0f;
    float lineH = tr->GetLineHeight(scale) * m_lineSpacing;
    float ascent = tr->GetBaselineOffset(scale);

    // 文本渲染矩形 = 以内容尺寸中心为中心的矩形（与节点锚点无关，世界中心为基准）
    float centerX = worldTransform.m[3][0];
    float centerY = worldTransform.m[3][1];
    float rectLeft = centerX - m_contentSize.x * 0.5f;
    float rectTop = centerY + m_contentSize.y * 0.5f;

    // 按内容宽度折行；行数组总高 = 行数 × 行距后的行高
    std::vector<std::string> lines = tr->WrapString(m_text, m_contentSize.x, scale);
    float totalH = static_cast<float>(lines.size()) * lineH;

    float firstBaseline = rectTop - lineH + ascent;
    if (m_vAlign == VAlign::Middle) {
        firstBaseline = centerY + ascent - totalH * 0.5f;
    } else if (m_vAlign == VAlign::Bottom) {
        firstBaseline = rectTop - totalH + ascent;
    }

    float cursorY = firstBaseline;
    for (const std::string& line : lines) {
        Vec2 lineSize = tr->MeasureString(line, scale);

        float x = rectLeft;
        if (m_hAlign == Align::Center) {
            x = rectLeft + (m_contentSize.x - lineSize.x) * 0.5f;
        } else if (m_hAlign == Align::Right) {
            x = rectLeft + m_contentSize.x - lineSize.x;
        }

        tr->RenderString(renderer, line, x, cursorY, scale, color, TextRenderer::Align::Left);
        cursorY += lineH;
    }
}
