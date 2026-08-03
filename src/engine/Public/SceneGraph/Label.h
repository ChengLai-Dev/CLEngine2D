#pragma once

#include "SceneGraph/Widget.h"
#include "TextRenderer.h"
#include <string>
#include <memory>

class Texture;

class Label : public Widget {
public:
    using Align = TextRenderer::Align;
    using VAlign = TextRenderer::VAlign;

    Label();
    virtual ~Label();

    void SetText(const std::string& text);
    const std::string& GetText() const;

    void SetFontSize(float size);
    float GetFontSize() const;

    void SetTextColor(const Vec4& color);
    const Vec4& GetTextColor() const;

    void SetBackground(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetBackground() const;

    // 水平对齐（默认 Center，单行文本与旧行为一致：整段居中）
    void SetHAlign(Align align);
    Align GetHAlign() const;

    // 垂直对齐（默认 Middle）
    void SetVAlign(VAlign align);
    VAlign GetVAlign() const;

    // 行距倍率（默认 1.0 = 单倍行距；对话文本建议 1.6）
    void SetLineSpacing(float spacing);
    float GetLineSpacing() const;

protected:
    void OnDraw(Renderer& renderer, const Mat4& worldTransform, float worldOpacity) override;

private:
    std::string m_text;
    float m_fontSize = 16.0f;
    Vec4 m_textColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    std::shared_ptr<Texture> m_background = nullptr;

    Align m_hAlign = Align::Center;
    VAlign m_vAlign = VAlign::Middle;
    float m_lineSpacing = 1.0f;
};
