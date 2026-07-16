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
    return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
}

void TabBar::OnRender(Renderer& renderer) {
    Color bgColor(0.06f, 0.06f, 0.08f, 1.0f);

    Mat4 bg = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bg, Vec2(m_rectWidth, m_rectHeight), bgColor);

    float tabW = GetTabWidth();
    float tabH = m_rectHeight;
    float x = GetTabStartX();
    Color activeColor(0.1f, 0.1f, 0.13f, 1.0f);
    Color inactiveColor(0.07f, 0.07f, 0.1f, 1.0f);
    float activeTextColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float inactiveTextColor[4] = { 0.55f, 0.55f, 0.6f, 1.0f };
    Color closeBtnColor(0.6f, 0.6f, 0.65f, 1.0f);
    Color closeBtnHoverColor(0.9f, 0.3f, 0.3f, 1.0f);

    for (size_t i = 0; i < m_tabs.size(); ++i) {
        bool isActive = static_cast<int>(i) == m_activeTab;
        bool isHoverClose = static_cast<int>(i) == m_hoveredClose;

        float cx = x + tabW * 0.5f;
        float cy = tabH * 0.5f;
        Mat4 tabBg = Mat4::Translate(Vec3(cx, cy, 0.0f));
        renderer.DrawQuad(tabBg, Vec2(tabW - 2.0f, tabH - 2.0f),
                          isActive ? activeColor : inactiveColor);

        if (m_fontRenderer) {
            std::string label = m_tabs[i].name;
            if (m_tabs[i].dirty) {
                label += " *";
            }

            float textX = x + 8.0f;
            float textH = m_fontRenderer->GetLineHeight(1.0f);
            float base = m_fontRenderer->GetBaselineOffset(1.0f);
            m_fontRenderer->RenderString(renderer, label,
                textX, (tabH - textH) * 0.5f + base,
                1.0f, isActive ? activeTextColor : inactiveTextColor,
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
                1.0f, activeTextColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }

        x += tabW;
    }
}

bool TabBar::OnMouseEvent(const MouseEvent& event) {
    float localX = event.screenPos.x - m_rectLeft;
    float localY = event.screenPos.y - m_rectTop;

    switch (event.type) {
        case MouseEvent::Move: {
            if (event.button == MouseEvent::None) {
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
