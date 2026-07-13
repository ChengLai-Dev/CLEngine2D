#pragma once

#include <string>

class Renderer;
class TextRenderer;

class PropertyEditBox {
public:
    PropertyEditBox() = default;

    void SetValue(const std::string& v) { m_displayValue = v; }
    const std::string& GetValue() const { return m_buffer; }
    const std::string& GetOldValue() const { return m_oldValue; }

    void Activate(const std::string& initialValue);
    void Deactivate();
    void Revert();
    bool IsActive() const { return m_active; }
    bool IsCursorVisible() const { return m_active && m_cursorVisible; }

    void Draw(Renderer& renderer, TextRenderer* fontRenderer,
              float x, float y, float width, float height,
              bool isActive) const;

    void OnMouseDown(TextRenderer* fontRenderer, float clickLocalX,
                     float valueLeft, float valuePadding);
    void OnMouseDrag(TextRenderer* fontRenderer, float localX,
                     float valueLeft, float valuePadding);
    void OnMouseRelease();
    void OnUpdate(float deltaTime);

private:
    std::string m_displayValue;
    std::string m_buffer;
    std::string m_oldValue;
    int m_cursorPos = 0;
    int m_selStart = -1;
    bool m_active = false;
    float m_blinkTimer = 0.0f;
    bool m_cursorVisible = true;

    int GetCharIndexAtX(TextRenderer* fontRenderer, float localX,
                        float valLeft, float valPad) const;
    float GetCharX(TextRenderer* fontRenderer, int charIndex,
                   float valLeft, float valPad) const;
    int GetSelBegin() const;
    int GetSelEnd() const;
    bool HasSelection() const;
    void DeleteSelection();
};
