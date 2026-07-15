#include "SceneGraph/Button.h"
#include "SceneGraph/UISystem.h"
#include "Render/Texture.h"
#include "Render/Renderer.h"
#include "TextRenderer.h"

Button::Button() {
    m_focusable = true;
    m_touchEnabled = true;
}

Button::~Button() = default;

void Button::SetNormalImage(std::shared_ptr<Texture> tex) {
    m_normalTex = std::move(tex);
    if (!m_pressedTex) {
        m_pressedTex = m_normalTex;
    }
    if (!m_disabledTex) {
        m_disabledTex = m_normalTex;
    }
}

void Button::SetPressedImage(std::shared_ptr<Texture> tex) {
    m_pressedTex = std::move(tex);
}

void Button::SetDisabledImage(std::shared_ptr<Texture> tex) {
    m_disabledTex = std::move(tex);
}

void Button::SetText(const std::string& text) {
    m_text = text;
}

const std::string& Button::GetText() const {
    return m_text;
}

void Button::SetTextColor(const Vec4& color) {
    m_textColor = color;
}

const Vec4& Button::GetTextColor() const {
    return m_textColor;
}

void Button::SetFontSize(float size) {
    m_fontSize = size;
}

float Button::GetFontSize() const {
    return m_fontSize;
}

void Button::OnClicked(ClickCallback cb) {
    m_onClicked = std::move(cb);
}

void Button::SetInteractable(bool interactable) {
    m_interactable = interactable;
    m_state = interactable ? State::NORMAL : State::DISABLED;
}

bool Button::IsInteractable() const {
    return m_interactable;
}

void Button::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
    std::shared_ptr<Texture> tex = m_normalTex;
    if (m_state == State::PRESSED && m_pressedTex) {
        tex = m_pressedTex;
    }
    if (m_state == State::DISABLED) {
        tex = m_disabledTex ? m_disabledTex : m_normalTex;
    }

    Color finalColor(m_color.x, m_color.y, m_color.z, m_color.w * worldOpacity);
    renderer.DrawQuad(worldTransform, m_contentSize, finalColor, tex.get());

    TextRenderer* tr = UISystem::GetInstance().GetFontRenderer();
    if (!tr || !tr->IsLoaded() || m_text.empty()) return;

    float color[4] = {
        m_textColor.x, m_textColor.y,
        m_textColor.z, m_textColor.w * worldOpacity
    };

    Vec2 textSize = tr->MeasureString(m_text, 1.0f);
    float scale = m_fontSize / 14.0f;

    float centerX = worldTransform.m[3][0];
    float centerY = worldTransform.m[3][1];
    float textX = centerX - textSize.x * scale * 0.5f;
    float ascent = tr->GetBaselineOffset(scale);
    float textY = centerY + ascent - textSize.y * scale * 0.5f;

    tr->RenderString(renderer, m_text, textX, textY, scale, color, TextRenderer::Align::Left);
}

void Button::OnTouchStartedEvent(const Vec2& pos) {
    if (!m_interactable) return;
    m_state = State::PRESSED;
    Widget::OnTouchStartedEvent(pos);
}

void Button::OnTouchEndedEvent(const Vec2& pos) {
    if (m_state == State::PRESSED) {
        m_state = State::NORMAL;
        if (m_onClicked) {
            m_onClicked(this);
        }
    }
    Widget::OnTouchEndedEvent(pos);
}
