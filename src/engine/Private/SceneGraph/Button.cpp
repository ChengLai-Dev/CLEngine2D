#include "SceneGraph/Button.h"
#include "SceneGraph/Sprite.h"
#include "Render/Texture.h"
#include "Render/Renderer.h"

Button::Button() {
    m_focusable = true;
    m_touchEnabled = true;

    auto normal = std::make_unique<Sprite>();
    m_normalRenderer = normal.get();
    AddChild(std::move(normal));

    auto pressed = std::make_unique<Sprite>();
    m_pressedRenderer = pressed.get();
    m_pressedRenderer->SetVisible(false);
    AddChild(std::move(pressed));

    auto disabled = std::make_unique<Sprite>();
    m_disabledRenderer = disabled.get();
    m_disabledRenderer->SetVisible(false);
    AddChild(std::move(disabled));

    auto label = std::make_unique<Label>();
    m_label = label.get();
    AddChild(std::move(label));
}

Button::~Button() = default;

void Button::SetNormalImage(std::shared_ptr<Texture> tex) {
    m_normalTex = std::move(tex);
    m_normalRenderer->SetTexture(m_normalTex);
    if (!m_pressedTex) {
        m_pressedRenderer->SetTexture(m_normalTex);
    }
    if (!m_disabledTex) {
        m_disabledRenderer->SetTexture(m_normalTex);
    }
    UpdateVisualState();
}

void Button::SetPressedImage(std::shared_ptr<Texture> tex) {
    m_pressedTex = std::move(tex);
    m_pressedRenderer->SetTexture(m_pressedTex);
}

void Button::SetDisabledImage(std::shared_ptr<Texture> tex) {
    m_disabledTex = std::move(tex);
    m_disabledRenderer->SetTexture(m_disabledTex);
}

void Button::SetText(const std::string& text) {
    m_label->SetText(text);
}

void Button::SetTextColor(const Vec4& color) {
    m_label->SetTextColor(color);
}

void Button::OnClicked(ClickCallback cb) {
    m_onClicked = std::move(cb);
}

void Button::SetInteractable(bool interactable) {
    m_interactable = interactable;
    m_state = interactable ? State::NORMAL : State::DISABLED;
    UpdateVisualState();
}

bool Button::IsInteractable() const {
    return m_interactable;
}

void Button::OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) {
}

void Button::OnTouchStartedEvent(const Vec2& pos) {
    if (!m_interactable) return;
    m_state = State::PRESSED;
    UpdateVisualState();
    Widget::OnTouchStartedEvent(pos);
}

void Button::OnTouchEndedEvent(const Vec2& pos) {
    if (m_state == State::PRESSED) {
        m_state = State::NORMAL;
        UpdateVisualState();
        if (m_onClicked) {
            m_onClicked(this);
        }
    }
    Widget::OnTouchEndedEvent(pos);
}

void Button::UpdateVisualState() {
    m_normalRenderer->SetVisible(m_state == State::NORMAL);
    m_pressedRenderer->SetVisible(m_state == State::PRESSED);
    m_disabledRenderer->SetVisible(m_state == State::DISABLED);
}
