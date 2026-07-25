#include "TabBar.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

TabBar::TabBar() {
    m_rectWidth = 800.0f;
    m_rectHeight = TAB_BAR_HEIGHT;
}

void TabBar::SetTabs(const std::vector<TabInfo>& tabs) {
    m_tabs = tabs;
}

void TabBar::SetActiveTab(int index) {
    m_activeTab = index;
}

HitRect TabBar::GetHitRect() const {
    HitRect r = { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
    if (m_tooltipTabIndex >= 0) {
        r.h += 40.0f;
    }
    return r;
}

void TabBar::OnUpdate(float deltaTime) {
    if (m_hoveredTab >= 0 && m_hoveredClose < 0) {
        m_hoverTimer += deltaTime;
        if (m_hoverTimer >= TOOLTIP_DELAY) {
            m_tooltipTabIndex = m_hoveredTab;
        }
    } else {
        m_hoverTimer = 0.0f;
        m_tooltipTabIndex = -1;
    }
}

static std::string TruncateText(TextRenderer* font, const std::string& text,
                                float maxWidth, float scale) {
    Vec2 fullSize = font->MeasureString(text, scale);
    if (fullSize.x <= maxWidth) return text;

    std::string suffix = "...";
    float suffixW = font->MeasureString(suffix, scale).x;
    float avail = maxWidth - suffixW;
    if (avail <= 0.0f) return suffix;

    for (size_t n = 1; n < text.size(); ++n) {
        std::string sub = text.substr(0, n);
        float w = font->MeasureString(sub, scale).x;
        if (w > avail) {
            return text.substr(0, n - 1) + suffix;
        }
    }
    return text + suffix;
}

void TabBar::OnRender(Renderer& renderer) {
    Color bgColor(0.06f, 0.06f, 0.08f, 1.0f);

    Mat4 bg = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bg, Vec2(m_rectWidth, m_rectHeight), bgColor);

    float tabW = GetTabWidth();
    float tabH = m_rectHeight;
    float x = GetTabStartX();
    float scale = 1.0f;
    Color activeColor(0.1f, 0.1f, 0.13f, 1.0f);
    Color inactiveColor(0.07f, 0.07f, 0.1f, 1.0f);
    Color extActiveColor(0.14f, 0.14f, 0.18f, 1.0f);
    Color extInactiveColor(0.11f, 0.11f, 0.15f, 1.0f);
    float activeTextColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float inactiveTextColor[4] = { 0.55f, 0.55f, 0.6f, 1.0f };
    float extActiveTextColor[4] = { 0.3f, 0.7f, 1.0f, 1.0f };
    float extInactiveTextColor[4] = { 0.25f, 0.5f, 0.75f, 1.0f };
    Color closeBtnColor(0.6f, 0.6f, 0.65f, 1.0f);
    Color closeBtnHoverColor(0.9f, 0.3f, 0.3f, 1.0f);

    for (size_t i = 0; i < m_tabs.size(); ++i) {
        bool isActive = static_cast<int>(i) == m_activeTab;
        bool isHoverClose = static_cast<int>(i) == m_hoveredClose;

        bool isExt = m_tabs[i].external;
        float cx = x + tabW * 0.5f;
        float cy = tabH * 0.5f;
        Mat4 tabBg = Mat4::Translate(Vec3(cx, cy, 0.0f));

        if (isExt) {
            renderer.DrawQuad(tabBg, Vec2(tabW - 2.0f, tabH - 2.0f),
                              isActive ? extActiveColor : extInactiveColor);
        } else {
            renderer.DrawQuad(tabBg, Vec2(tabW - 2.0f, tabH - 2.0f),
                              isActive ? activeColor : inactiveColor);
        }

        if (m_fontRenderer) {
            std::string label;
            if (isExt) {
                label = "[External] ";
            }
            label += m_tabs[i].name;
            if (m_tabs[i].dirty) {
                label += " *";
            }

            std::string display = TruncateText(m_fontRenderer, label,
                                               MAX_TEXT_WIDTH, scale);

            float textX = x + TAB_TEXT_LEFT;
            float textH = m_fontRenderer->GetLineHeight(scale);
            float base = m_fontRenderer->GetBaselineOffset(scale);
            const float* tc;
            if (isExt) {
                tc = isActive ? extActiveTextColor : extInactiveTextColor;
            } else {
                tc = isActive ? activeTextColor : inactiveTextColor;
            }
            m_fontRenderer->RenderString(renderer, display,
                textX, (tabH - textH) * 0.5f + base,
                scale, tc,
                TextRenderer::Align::Left);
        }

        // Close button
        float btnX = x + tabW - CLOSE_BTN_SIZE - 4.0f;
        float btnY = (tabH - CLOSE_BTN_SIZE) * 0.5f;
        float btnCx = btnX + CLOSE_BTN_SIZE * 0.5f;
        float btnCy = btnY + CLOSE_BTN_SIZE * 0.5f;

        if (isHoverClose) {
            Mat4 btnBg = Mat4::Translate(Vec3(btnCx, btnCy, 0.0f));
            renderer.DrawQuad(btnBg, Vec2(CLOSE_BTN_SIZE, CLOSE_BTN_SIZE),
                              closeBtnColor);
        }

        if (m_fontRenderer) {
            m_fontRenderer->RenderStringInRect(renderer, "x",
                btnX, btnY, CLOSE_BTN_SIZE, CLOSE_BTN_SIZE,
                scale, activeTextColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }

        x += tabW;
    }

    // Tooltip
    if (m_tooltipTabIndex >= 0 && m_tooltipTabIndex < static_cast<int>(m_tabs.size())) {
        if (m_fontRenderer) {
            const std::string& tipText = m_tabs[m_tooltipTabIndex].filePath;
            float lineH = m_fontRenderer->GetLineHeight(scale);
            float textW = m_fontRenderer->MeasureString(tipText, scale).x;
            float tipH = lineH + 16.0f;
            float tipW = textW + 12.0f;
            float tipX = m_mouseX;
            if (tipX + tipW > m_rectLeft + m_rectWidth)
                tipX = m_rectLeft + m_rectWidth - tipW;
            if (tipX < m_rectLeft) tipX = m_rectLeft;
            float tipY = m_rectTop + tabH + 2.0f;

            Color tipBg(0.05f, 0.05f, 0.08f, 0.95f);
            Color tipBorder(0.25f, 0.25f, 0.3f, 1.0f);
            Mat4 tipBgM = Mat4::Translate(Vec3(tipX + tipW * 0.5f, tipY + tipH * 0.5f, 0.0f));
            renderer.DrawQuad(tipBgM, Vec2(tipW, tipH), tipBg);
            renderer.DrawQuad(tipBgM, Vec2(tipW, 1.0f), tipBorder);

            float base = m_fontRenderer->GetBaselineOffset(scale);
            m_fontRenderer->RenderString(renderer, tipText,
                tipX + 6.0f, tipY + (tipH - lineH) * 0.5f + base,
                scale, activeTextColor,
                TextRenderer::Align::Left);
        }
    }
}

bool TabBar::OnMouseEvent(const MouseEvent& event) {
    float localX = event.screenPos.x - m_rectLeft;
    float localY = event.screenPos.y - m_rectTop;

    switch (event.type) {
        case MouseEvent::Move: {
            m_mouseX = event.screenPos.x;
            m_mouseY = event.screenPos.y;
            if (event.button == MouseEvent::None) {
                int newTab = HitTestTab(localX);
                bool movedFar = std::abs(m_mouseX - m_lastHoverX) >= MOVE_THRESHOLD ||
                                std::abs(m_mouseY - m_lastHoverY) >= MOVE_THRESHOLD;

                if (newTab != m_hoveredTab) {
                    m_hoveredTab = newTab;
                    m_hoverTimer = 0.0f;
                    m_tooltipTabIndex = -1;
                    m_lastHoverX = m_mouseX;
                    m_lastHoverY = m_mouseY;
                } else if (m_tooltipTabIndex >= 0 && movedFar) {
                    m_tooltipTabIndex = -1;
                    m_hoverTimer = 0.0f;
                    m_lastHoverX = m_mouseX;
                    m_lastHoverY = m_mouseY;
                } else if (m_tooltipTabIndex < 0 && movedFar) {
                    m_hoverTimer = 0.0f;
                    m_lastHoverX = m_mouseX;
                    m_lastHoverY = m_mouseY;
                }

                m_hoveredClose = HitTestClose(localX, localY);
                return true;
            }
            return false;
        }

        case MouseEvent::Press: {
            if (event.button != MouseEvent::Left) return false;
            if (localY < 0.0f || localY >= m_rectHeight) return false;

            int closeIdx = HitTestClose(localX, localY);
            if (closeIdx >= 0) {
                if (m_onTabClose) m_onTabClose(closeIdx);
                return true;
            }

            int tabIdx = HitTestTab(localX);
            if (tabIdx >= 0 && tabIdx != m_activeTab) {
                if (m_onTabSwitch) m_onTabSwitch(tabIdx);
                return true;
            }
            return false;
        }

        default:
            return false;
    }
}

int TabBar::HitTestTab(float mx) const {
    float x = GetTabStartX();
    float tabW = GetTabWidth();

    for (size_t i = 0; i < m_tabs.size(); ++i) {
        if (mx >= x && mx < x + tabW) {
            return static_cast<int>(i);
        }
        x += tabW;
    }
    return -1;
}

int TabBar::HitTestClose(float mx, float my) const {
    float x = GetTabStartX();
    float tabW = GetTabWidth();

    for (size_t i = 0; i < m_tabs.size(); ++i) {
        float btnX = x + tabW - CLOSE_BTN_SIZE - 4.0f;
        float btnY = (m_rectHeight - CLOSE_BTN_SIZE) * 0.5f;
        if (mx >= btnX && mx < btnX + CLOSE_BTN_SIZE &&
            my >= btnY && my < btnY + CLOSE_BTN_SIZE) {
            return static_cast<int>(i);
        }
        x += tabW;
    }
    return -1;
}

void TabBar::OnTabSwitch(TabSwitchCallback cb) {
    m_onTabSwitch = std::move(cb);
}

void TabBar::OnTabClose(TabCloseCallback cb) {
    m_onTabClose = std::move(cb);
}
