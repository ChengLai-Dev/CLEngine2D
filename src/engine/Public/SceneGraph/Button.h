#pragma once

#include "SceneGraph/Widget.h"
#include <memory>
#include <functional>

class Texture;

class Button : public Widget {
public:
    Button();
    virtual ~Button();

    using ClickCallback = std::function<void(Button*)>;

    void SetNormalImage(std::shared_ptr<Texture> tex);
    void SetPressedImage(std::shared_ptr<Texture> tex);
    void SetDisabledImage(std::shared_ptr<Texture> tex);

    void SetText(const std::string& text);
    void SetTextColor(const Vec4& color);

    void OnClicked(ClickCallback cb);

    void SetInteractable(bool interactable);
    bool IsInteractable() const;

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;
    void OnTouchStartedEvent(const Vec2& pos) override;
    void OnTouchEndedEvent(const Vec2& pos) override;

private:
    enum class State { NORMAL, PRESSED, DISABLED };

    State m_state = State::NORMAL;

    std::shared_ptr<Texture> m_normalTex;
    std::shared_ptr<Texture> m_pressedTex;
    std::shared_ptr<Texture> m_disabledTex;

    std::string m_text;
    Vec4 m_textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    float m_fontSize = 16.0f;

    ClickCallback m_onClicked;
    bool m_interactable = true;
};
