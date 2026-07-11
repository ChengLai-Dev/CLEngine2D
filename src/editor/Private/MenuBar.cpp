#include "MenuBar.h"
#include <Render/Renderer.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <TextRenderer.h>

static const char* FILE_LABELS[] = { "New Project", "Open Project", "Save Project" };
static const MenuBarAction FILE_ACTIONS[] = {
    MenuBarAction::FILE_NEW, MenuBarAction::FILE_OPEN, MenuBarAction::FILE_SAVE
};

static const char* EDIT_LABELS[] = { "Undo", "Redo", "Delete" };
static const MenuBarAction EDIT_ACTIONS[] = {
    MenuBarAction::EDIT_UNDO, MenuBarAction::EDIT_REDO, MenuBarAction::EDIT_DELETE
};

static const char* MENU_LABELS[] = { "File", "Edit" };
static const int MENU_COUNT = 2;

MenuBar::MenuBar() {
    m_rectWidth = 1280.0f;
    m_rectHeight = 24.0f;
}

MenuBar::~MenuBar() = default;

void MenuBar::OnAction(ActionCallback cb) {
    m_onAction = std::move(cb);
}

void MenuBar::DrawDropdown(Renderer& renderer, float dropX, float dropY,
                           const char* const* items, const MenuBarAction* actions, int count) {
    (void)actions;
    Color bgColor(0.0f, 0.0f, 0.0f, 1.0f);
    Color itemColor(0.1f, 0.1f, 0.1f, 1.0f);
    float textColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    float itemH = 22.0f;
    float menuW = 160.0f;
    float totalH = static_cast<float>(count) * itemH;

    Mat4 bg = Mat4::Translate(Vec3(dropX + menuW * 0.5f, dropY + totalH * 0.5f, 0.0f));
    renderer.DrawQuad(bg, Vec2(menuW, totalH), bgColor);

    for (int i = 0; i < count; ++i) {
        float iy = dropY + static_cast<float>(i) * itemH;
        Mat4 itemBg = Mat4::Translate(Vec3(dropX + menuW * 0.5f, iy + itemH * 0.5f, 0.0f));
        renderer.DrawQuad(itemBg, Vec2(menuW - 2.0f, itemH - 1.0f), itemColor);

        if (m_fontRenderer && items[i]) {
            float textH = m_fontRenderer->GetLineHeight(1.0f);
            float base = m_fontRenderer->GetBaselineOffset(1.0f);
            m_fontRenderer->RenderString(renderer, items[i],
                dropX + 8.0f, iy + (itemH - textH) * 0.5f + base,
                1.0f, textColor, TextRenderer::Align::Left);
        }
    }
}

void MenuBar::OnRender(Renderer& renderer) {
    Color bgColor(0.0f, 0.0f, 0.0f, 1.0f);
    float textColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    Mat4 bg = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bg, Vec2(m_rectWidth, m_rectHeight), bgColor);

    float labelX = 12.0f;
    float labelW = 60.0f;
    for (int i = 0; i < MENU_COUNT; ++i) {
        Mat4 labelBg = Mat4::Translate(Vec3(labelX + labelW * 0.5f, m_rectHeight * 0.5f, 0.0f));
        renderer.DrawQuad(labelBg, Vec2(labelW, m_rectHeight), bgColor);

        if (m_fontRenderer) {
            m_fontRenderer->RenderStringInRect(renderer, MENU_LABELS[i],
                labelX, 0.0f, labelW, m_rectHeight,
                1.0f, textColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }

        labelX += labelW;
    }

    if (m_openMenu == 0) {
        DrawDropdown(renderer, 0.0f, m_rectHeight, FILE_LABELS, FILE_ACTIONS, 3);
    } else if (m_openMenu == 1) {
        DrawDropdown(renderer, 60.0f, m_rectHeight, EDIT_LABELS, EDIT_ACTIONS, 3);
    }
}

IEditorPanel::HitRect MenuBar::GetHitRect() const {
    HitRect r = { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
    if (m_openMenu >= 0) {
        r.h += 3.0f * 22.0f;
    }
    return r;
}

bool MenuBar::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    if (event.type != MouseEvent::Down || event.button != MouseEvent::Left) return false;
    return HandleClick(event.screenPos.x, event.screenPos.y);
}

bool MenuBar::HandleClick(float mx, float my) {
    bool inBar = mx >= m_rectLeft && mx < m_rectLeft + m_rectWidth &&
                 my >= m_rectTop && my < m_rectTop + m_rectHeight;

    if (m_openMenu >= 0) {
        const MenuBarAction* actions = (m_openMenu == 0) ? FILE_ACTIONS : EDIT_ACTIONS;
        int count = 3;
        float menuW = 160.0f;
        float itemH = 22.0f;
        float dropX = (m_openMenu == 0) ? 0.0f : 60.0f;
        float dropY = m_rectTop + m_rectHeight;

        for (int i = 0; i < count; ++i) {
            float ix = dropX;
            float iy = dropY + static_cast<float>(i) * itemH;
            if (mx >= ix && mx < ix + menuW && my >= iy && my < iy + itemH) {
                if (m_onAction) m_onAction(actions[i]);
                m_openMenu = -1;
                return true;
            }
        }

        if (!inBar || my >= m_rectTop + m_rectHeight) {
            m_openMenu = -1;
            return true;
        }
    }

    if (inBar) {
        float labelX = 12.0f;
        float labelW = 60.0f;
        for (int i = 0; i < MENU_COUNT; ++i) {
            if (mx >= m_rectLeft + labelX && mx < m_rectLeft + labelX + labelW &&
                my >= m_rectTop && my < m_rectTop + m_rectHeight) {
                m_openMenu = (m_openMenu == i) ? -1 : i;
                return true;
            }
            labelX += labelW;
        }
    }

    return false;
}
