#pragma once

#include "SceneGraph/Widget.h"
#include <string>
#include <memory>

class Texture;

class Label : public Widget {
public:
    Label();
    virtual ~Label();

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetFontSize(float size);
    float GetFontSize() const;

    void SetTextColor(const Vec4& color);
    const Vec4& GetTextColor() const;

    void SetBackground(std::shared_ptr<Texture> texture);

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    std::string m_text;
    float m_fontSize = 16.0f;
    Vec4 m_textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    std::shared_ptr<Texture> m_background = nullptr;
};
