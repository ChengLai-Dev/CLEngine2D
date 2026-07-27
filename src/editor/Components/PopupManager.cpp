#include "PopupManager.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

void PopupManager::Open(float screenX, float screenY,
                        const PopupMenu::Item* items, int count,
                        std::function<void(int)> onSelected,
                        std::function<void()> onDismissed) {
    if (m_open) {
        if (m_onDismissed) m_onDismissed();
    }
    m_x = screenX;
    m_y = screenY;
    m_items.assign(items, items + count);
    m_onSelected = std::move(onSelected);
    m_onDismissed = std::move(onDismissed);
    m_open = true;
}

void PopupManager::Close() {
    if (!m_open) return;
    m_open = false;
    m_items.clear();
    auto dismissed = std::move(m_onDismissed);
    m_onSelected = nullptr;
    if (dismissed) dismissed();
}

bool PopupManager::IsOpen() const {
    return m_open;
}

bool PopupManager::OnMouseClick(float mx, float my) {
    if (!m_open) return false;

    int count = static_cast<int>(m_items.size());
    int hit = PopupMenu::HitTest(mx, my, m_x, m_y, count, itemWidth, itemHeight);

    if (hit >= 0) {
        auto selected = std::move(m_onSelected);
        auto dismissed = std::move(m_onDismissed);
        m_open = false;
        m_items.clear();
        if (selected) selected(m_items[hit].data);
        return true;
    }

    Close();
    return false;
}

void PopupManager::OnKeyPress(KeyCode key) {
    if (!m_open) return;
    if (key == KeyCode::Escape) {
        Close();
    }
}

void PopupManager::Draw(Renderer& renderer, TextRenderer* font) const {
    if (!m_open) return;
    int count = static_cast<int>(m_items.size());
    PopupMenu::Draw(renderer, font, m_x, m_y, m_items.data(), count, itemWidth, itemHeight);
}

HitRect PopupManager::GetHitRect() const {
    if (!m_open) return { 0.0f, 0.0f, 0.0f, 0.0f };
    int count = static_cast<int>(m_items.size());
    return PopupMenu::GetHitRect(m_x, m_y, count, itemWidth, itemHeight);
}
