#pragma once

#include "PopupMenu.h"
#include <functional>
#include <vector>
#include <Input/InputCodes.h>

class Renderer;
class TextRenderer;

class PopupManager {
public:
    PopupManager() = default;

    void Open(float screenX, float screenY,
              const PopupMenu::Item* items, int count,
              std::function<void(int)> onSelected = nullptr,
              std::function<void()> onDismissed = nullptr);
    void Close();
    bool IsOpen() const;

    bool OnMouseClick(float mx, float my);
    void OnKeyPress(KeyCode key);
    void Draw(Renderer& renderer, TextRenderer* font) const;
    HitRect GetHitRect() const;

    float itemWidth = 160.0f;
    float itemHeight = 22.0f;

private:
    bool m_open = false;
    float m_x = 0.0f, m_y = 0.0f;
    std::vector<PopupMenu::Item> m_items;
    std::function<void(int)> m_onSelected;
    std::function<void()> m_onDismissed;
};
