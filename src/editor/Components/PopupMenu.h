#pragma once

#include "HitRect.h"
#include <string>
#include <functional>

class Renderer;
class TextRenderer;

class PopupMenu {
public:
    struct Item {
        std::string label;
        int data = 0;
    };

    static void Draw(Renderer& renderer, TextRenderer* font,
                     float screenX, float screenY,
                     const Item* items, int count,
                     float itemWidth = 160.0f, float itemHeight = 22.0f);

    static int HitTest(float mx, float my,
                       float menuScreenX, float menuScreenY,
                       int count,
                       float itemWidth = 160.0f, float itemHeight = 22.0f);

    static HitRect GetHitRect(float menuScreenX, float menuScreenY,
                              int count,
                              float itemWidth = 160.0f, float itemHeight = 22.0f);
};

struct PopupRequest {
    float screenX = 0.0f;
    float screenY = 0.0f;
    const PopupMenu::Item* items = nullptr;
    int count = 0;
    std::function<void(int)> onSelected;
    std::function<void()> onDismissed;
};
