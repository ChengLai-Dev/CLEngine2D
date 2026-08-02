#include "PopupMenu.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <Input/InputCodes.h>
#include <Input/RawInput.h>

void PopupMenu::Open(Vec2 pos, std::vector<Item> items,
                     std::function<void(int)> onSelected,
                     std::function<void()> onDismissed) {
    m_pos = pos;
    m_items = std::move(items);
    m_onSelected = std::move(onSelected);
    m_onDismissed = std::move(onDismissed);
    m_open = true;
}

void PopupMenu::Close() {
    m_open = false;
    m_items.clear();
}

bool PopupMenu::IsOpen() const {
    return m_open;
}

bool PopupMenu::IsVisible() const {
    return m_open;
}

void PopupMenu::OnUpdate(float deltaTime) {
    (void)deltaTime;
}

void PopupMenu::OnRender(Renderer& renderer) {
    if (!m_open) return;

    int count = static_cast<int>(m_items.size());
    float totalH = static_cast<float>(count) * itemHeight;

    renderer.DrawQuad(
        Vec3(m_pos.x + itemWidth * 0.5f, m_pos.y + totalH * 0.5f, 0.0f),
        Vec3(itemWidth, totalH, 1.0f),
        Color(0.0f, 0.0f, 0.0f, 1.0f));

    if (m_fontRenderer) {
        float txtColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
        float textH = m_fontRenderer->GetLineHeight(1.0f);
        float base = m_fontRenderer->GetBaselineOffset(1.0f);
        for (int i = 0; i < count; ++i) {
            float iy = m_pos.y + static_cast<float>(i) * itemHeight;
            renderer.DrawQuad(
                Vec3(m_pos.x + itemWidth * 0.5f, iy + itemHeight * 0.5f, 0.0f),
                Vec3(itemWidth - 2.0f, itemHeight - 1.0f, 1.0f),
                Color(0.1f, 0.1f, 0.1f, 1.0f));

            m_fontRenderer->RenderString(renderer, m_items[i].label.c_str(),
                m_pos.x + 8.0f, iy + (itemHeight - textH) * 0.5f + base,
                1.0f, txtColor, TextRenderer::Align::Left);
        }
    }
}

bool PopupMenu::OnMouseEvent(const MouseEvent& event) {
    if (!m_open) return false;
    if (event.type != MouseEvent::Press) return false;
    if (event.button != MouseEvent::Left && event.button != MouseEvent::Right) return false;

    int hit = HitTest(event.screenPos);
    if (hit >= 0) {
        int data = m_items[hit].data;
        Close();
        if (m_onSelected) m_onSelected(data);
        return true;
    }

    Close();
    if (m_onDismissed) m_onDismissed();
    return false;
}

int PopupMenu::HitTest(Vec2 pos) const {
    int count = static_cast<int>(m_items.size());
    for (int i = 0; i < count; ++i) {
        float ix = m_pos.x;
        float iy = m_pos.y + static_cast<float>(i) * itemHeight;
        if (pos.x >= ix && pos.x < ix + itemWidth &&
            pos.y >= iy && pos.y < iy + itemHeight) {
            return i;
        }
    }
    return -1;
}
