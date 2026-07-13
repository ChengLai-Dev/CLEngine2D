#include "PropertyEditBox.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <algorithm>
#include <cstdlib>

void PropertyEditBox::Activate(const std::string& initialValue) {
    m_buffer = initialValue;
    m_oldValue = m_buffer;
    m_cursorPos = static_cast<int>(m_buffer.length());
    m_selStart = 0;
    m_active = true;
    m_blinkTimer = 0.0f;
    m_cursorVisible = true;
}

void PropertyEditBox::Deactivate() {
    m_buffer.clear();
    m_oldValue.clear();
    m_cursorPos = 0;
    m_selStart = -1;
    m_active = false;
    m_blinkTimer = 0.0f;
    m_cursorVisible = false;
}

void PropertyEditBox::Revert() {
    m_buffer = m_oldValue;
    m_cursorPos = static_cast<int>(m_buffer.length());
    m_selStart = -1;
}

void PropertyEditBox::Draw(Renderer& renderer, TextRenderer* fontRenderer,
                           float x, float y, float width, float height,
                           bool isActive) const {
    if (!fontRenderer) return;

    float bgColor[4];
    if (isActive) {
        bgColor[0] = 0.18f; bgColor[1] = 0.35f; bgColor[2] = 0.55f; bgColor[3] = 1.0f;
    } else {
        bgColor[0] = 0.13f; bgColor[1] = 0.13f; bgColor[2] = 0.15f; bgColor[3] = 1.0f;
    }

    float cx = x + width * 0.5f;
    float cy = y + height * 0.5f;
    Mat4 bgXform = Mat4::Translate(Vec3(cx, cy, 0.0f));
    renderer.DrawQuad(bgXform, Vec2(width, height - 2.0f),
                      Color(bgColor[0], bgColor[1], bgColor[2], bgColor[3]));

    const std::string& text = isActive ? m_buffer : m_displayValue;
    float valX = x + 6.0f;

    if (isActive && HasSelection()) {
        int begin = GetSelBegin();
        int end = GetSelEnd();
        float selX1 = GetCharX(fontRenderer, begin, valX, 0);
        float selX2 = GetCharX(fontRenderer, end, valX, 0);
        float selW = selX2 - selX1;
        if (selW > 0.0f) {
            float selCenterX = selX1 + selW * 0.5f;
            float selColor[4] = { 0.25f, 0.50f, 0.80f, 0.5f };
            Mat4 selXform = Mat4::Translate(Vec3(selCenterX, y + height * 0.5f, 0.0f));
            renderer.DrawQuad(selXform, Vec2(selW, height - 2.0f),
                              Color(selColor[0], selColor[1], selColor[2], selColor[3]));
        }
    }

    if (!text.empty()) {
        float textH = fontRenderer->GetLineHeight(1.0f);
        float base = fontRenderer->GetBaselineOffset(1.0f);
        float centerY = y + (height - textH) * 0.5f + base;

        float valColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };
        if (isActive) {
            valColor[0] = 1.0f; valColor[1] = 1.0f; valColor[2] = 1.0f;
        }
        fontRenderer->RenderString(renderer, text,
            valX, centerY, 1.0f, valColor, TextRenderer::Align::Left);
    }

    if (isActive && m_cursorVisible) {
        float cursorX = GetCharX(fontRenderer, m_cursorPos, valX, 0);
        renderer.DrawLine(
            Vec3(cursorX, y + 2.0f, 0.0f),
            Vec3(cursorX, y + height - 2.0f, 0.0f),
            Color(1.0f, 1.0f, 1.0f, 1.0f));
    }
}

void PropertyEditBox::OnMouseDown(TextRenderer* fontRenderer,
                                   float clickLocalX, float valueLeft,
                                   float valuePadding) {
    if (!m_active) return;

    m_cursorPos = GetCharIndexAtX(fontRenderer, clickLocalX, valueLeft, valuePadding);
    m_selStart = m_cursorPos;
    m_blinkTimer = 0.0f;
    m_cursorVisible = true;
}

void PropertyEditBox::OnMouseDrag(TextRenderer* fontRenderer,
                                   float localX, float valueLeft,
                                   float valuePadding) {
    if (!m_active) return;

    m_cursorPos = GetCharIndexAtX(fontRenderer, localX, valueLeft, valuePadding);
    m_blinkTimer = 0.0f;
    m_cursorVisible = true;
}

void PropertyEditBox::OnMouseRelease() {
    if (!m_active) return;

    if (m_selStart == m_cursorPos) {
        m_selStart = -1;
    }
}

void PropertyEditBox::OnUpdate(float deltaTime) {
    if (!m_active) return;

    std::string chars = RawInput::ConsumeCharBuffer();
    if (!chars.empty()) {
        if (HasSelection()) {
            DeleteSelection();
        } else if (m_selStart >= 0) {
            m_selStart = -1;
        }
        m_buffer.insert(static_cast<size_t>(m_cursorPos), chars);
        m_cursorPos += static_cast<int>(chars.length());
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    if (RawInput::IsKeyPressed(KeyCode::Left)) {
        if (HasSelection()) {
            m_cursorPos = GetSelBegin();
            m_selStart = -1;
        } else if (m_cursorPos > 0) {
            --m_cursorPos;
            m_selStart = -1;
        }
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    if (RawInput::IsKeyPressed(KeyCode::Right)) {
        if (HasSelection()) {
            m_cursorPos = GetSelEnd();
            m_selStart = -1;
        } else if (m_cursorPos < static_cast<int>(m_buffer.length())) {
            ++m_cursorPos;
            m_selStart = -1;
        }
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    if (RawInput::IsKeyPressed(KeyCode::Home)) {
        m_cursorPos = 0;
        m_selStart = -1;
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    if (RawInput::IsKeyPressed(KeyCode::End)) {
        m_cursorPos = static_cast<int>(m_buffer.length());
        m_selStart = -1;
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    if (RawInput::IsKeyPressed(KeyCode::Backspace)) {
        if (HasSelection()) {
            DeleteSelection();
        } else if (m_cursorPos > 0 && !m_buffer.empty()) {
            m_buffer.erase(static_cast<size_t>(m_cursorPos) - 1, 1);
            --m_cursorPos;
            m_selStart = -1;
        }
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    if (RawInput::IsKeyPressed(KeyCode::Delete)) {
        if (HasSelection()) {
            DeleteSelection();
        } else if (m_cursorPos < static_cast<int>(m_buffer.length())) {
            m_buffer.erase(static_cast<size_t>(m_cursorPos), 1);
            m_selStart = -1;
        }
        m_blinkTimer = 0.0f;
        m_cursorVisible = true;
    }

    m_blinkTimer += deltaTime;
    if (m_blinkTimer > 0.5f) {
        m_cursorVisible = !m_cursorVisible;
        m_blinkTimer = 0.0f;
    }
}

int PropertyEditBox::GetCharIndexAtX(TextRenderer* fontRenderer,
                                     float localX, float valLeft,
                                     float valPad) const {
    if (!fontRenderer) return 0;
    float valX = valLeft + valPad;
    float clickOffset = localX - valX;
    if (clickOffset <= 0.0f) return 0;

    const std::string& text = m_buffer;
    if (text.empty()) return 0;

    float cumulativeAdvance = 0.0f;
    for (size_t i = 0; i < text.length(); ++i) {
        unsigned char c = static_cast<unsigned char>(text[i]);
        float adv = fontRenderer->MeasureString(text.substr(i, 1), 1.0f).x;
        float bear = fontRenderer->GetGlyphBearingX(c, 1.0f);
        float boundary = cumulativeAdvance + (bear + adv) / 2.0f;
        if (clickOffset < boundary) return static_cast<int>(i);
        cumulativeAdvance += adv;
    }
    return static_cast<int>(text.length());
}

float PropertyEditBox::GetCharX(TextRenderer* fontRenderer, int charIndex,
                                 float valLeft, float valPad) const {
    if (!fontRenderer) return valLeft + valPad;
    const std::string& text = m_buffer;
    float x = valLeft + valPad;
    if (!text.empty()) {
        x += fontRenderer->GetGlyphBearingX(static_cast<unsigned char>(text[0]), 1.0f);
    }
    if (charIndex > 0) {
        size_t len = (std::min)(static_cast<size_t>(charIndex), text.length());
        Vec2 beforeSize = fontRenderer->MeasureString(text.substr(0, len), 1.0f);
        x += beforeSize.x;
    }
    return x;
}

int PropertyEditBox::GetSelBegin() const {
    return (std::min)(m_selStart, m_cursorPos);
}

int PropertyEditBox::GetSelEnd() const {
    return (std::max)(m_selStart, m_cursorPos);
}

bool PropertyEditBox::HasSelection() const {
    return m_selStart >= 0 && m_selStart != m_cursorPos;
}

void PropertyEditBox::DeleteSelection() {
    if (!HasSelection()) return;
    int begin = GetSelBegin();
    int end = GetSelEnd();
    m_buffer.erase(static_cast<size_t>(begin), static_cast<size_t>(end - begin));
    m_cursorPos = begin;
    m_selStart = -1;
}
