#include "PopupMenu.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

void PopupMenu::Draw(Renderer& renderer, TextRenderer* font,
                     float screenX, float screenY,
                     const Item* items, int count,
                     float itemWidth, float itemHeight) {
    float totalH = static_cast<float>(count) * itemHeight;

    renderer.DrawQuad(
        Mat4::Translate(Vec3(screenX + itemWidth * 0.5f, screenY + totalH * 0.5f, 0.0f)),
        Vec2(itemWidth, totalH),
        Color(0.0f, 0.0f, 0.0f, 1.0f));

    float txtColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    for (int i = 0; i < count; ++i) {
        float iy = screenY + static_cast<float>(i) * itemHeight;
        renderer.DrawQuad(
            Mat4::Translate(Vec3(screenX + itemWidth * 0.5f, iy + itemHeight * 0.5f, 0.0f)),
            Vec2(itemWidth - 2.0f, itemHeight - 1.0f),
            Color(0.1f, 0.1f, 0.1f, 1.0f));

        if (font) {
            float textH = font->GetLineHeight(1.0f);
            float base = font->GetBaselineOffset(1.0f);
            font->RenderString(renderer, items[i].label.c_str(),
                screenX + 8.0f, iy + (itemHeight - textH) * 0.5f + base,
                1.0f, txtColor, TextRenderer::Align::Left);
        }
    }
}

int PopupMenu::HitTest(float mx, float my,
                       float menuScreenX, float menuScreenY,
                       int count,
                       float itemWidth, float itemHeight) {
    for (int i = 0; i < count; ++i) {
        float ix = menuScreenX;
        float iy = menuScreenY + static_cast<float>(i) * itemHeight;
        if (mx >= ix && mx < ix + itemWidth &&
            my >= iy && my < iy + itemHeight) {
            return i;
        }
    }
    return -1;
}

HitRect PopupMenu::GetHitRect(float menuScreenX, float menuScreenY,
                              int count,
                              float itemWidth, float itemHeight) {
    return { menuScreenX, menuScreenY, itemWidth, static_cast<float>(count) * itemHeight };
}
