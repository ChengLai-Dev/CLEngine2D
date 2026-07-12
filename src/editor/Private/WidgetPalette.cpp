#define NOMINMAX
#include "WidgetPalette.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <algorithm>

WidgetPalette::WidgetPalette() = default;
WidgetPalette::~WidgetPalette() = default;

void WidgetPalette::OnAction(ActionCallback cb) {
    m_onAction = std::move(cb);
}

int WidgetPalette::CalcButtonsPerRow(float panelWidth) const {
    float availableWidth = panelWidth - 2.0f * kPadding;
    int maxPerRow = static_cast<int>((availableWidth + kGap) / (kMinBtnWidth + kGap));
    return std::max(1, std::min(maxPerRow, kBtnCount));
}

int WidgetPalette::CalcRowCount(float panelWidth) const {
    int perRow = CalcButtonsPerRow(panelWidth);
    return (kBtnCount + perRow - 1) / perRow;
}

float WidgetPalette::CalcDesiredHeight(float panelWidth) const {
    int rows = CalcRowCount(panelWidth);
    return 2.0f * kPadding + static_cast<float>(rows) * kBtnHeight +
           static_cast<float>(rows - 1) * kGap;
}

HitRect WidgetPalette::GetButtonRect(int index) const {
    int numPerRow = CalcButtonsPerRow(m_rectWidth);
    float availableWidth = m_rectWidth - 2.0f * kPadding;
    float btnWidth = (availableWidth - static_cast<float>(numPerRow - 1) * kGap) /
                     static_cast<float>(numPerRow);

    int row = index / numPerRow;
    int col = index % numPerRow;

    float x = kPadding + static_cast<float>(col) * (btnWidth + kGap);
    float y = kPadding + static_cast<float>(row) * (kBtnHeight + kGap);

    return { x, y, btnWidth, kBtnHeight };
}

void WidgetPalette::OnRender(Renderer& renderer) {
    Color bgColor(0.08f, 0.08f, 0.1f, 1.0f);
    Color btnColor(0.2f, 0.25f, 0.3f, 1.0f);

    float renderHeight = std::max(m_rectHeight, CalcDesiredHeight(m_rectWidth));
    Mat4 bgTransform = Mat4::Translate(
        Vec3(m_rectWidth * 0.5f, renderHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, renderHeight), bgColor);

    static const char* btnLabels[] = { "Button", "Label", "Image", "Panel", "Layout" };

    for (int i = 0; i < kBtnCount; ++i) {
        HitRect r = GetButtonRect(i);
        Mat4 btnTransform = Mat4::Translate(
            Vec3(r.x + r.w * 0.5f, r.y + r.h * 0.5f, 0.0f));
        renderer.DrawQuad(btnTransform, Vec2(r.w - 2.0f, r.h), btnColor);

        if (m_fontRenderer) {
            float textColor[4] = { 0.85f, 0.85f, 0.85f, 1.0f };
            m_fontRenderer->RenderStringInRect(renderer, btnLabels[i],
                r.x, r.y, r.w, r.h,
                1.0f, textColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }
    }
}

bool WidgetPalette::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    switch (event.type) {
        case MouseEvent::Down: {
            if (event.button != MouseEvent::Left) return false;
            float localX = event.screenPos.x - m_rectLeft;
            float localY = event.screenPos.y - m_rectTop;
            for (int i = 0; i < kBtnCount; ++i) {
                HitRect r = GetButtonRect(i);
                if (r.Contains(localX, localY)) {
                    m_dragIndex = i;
                    return true;
                }
            }
            return false;
        }
        case MouseEvent::Move: {
            return m_dragIndex >= 0;
        }
        case MouseEvent::Up: {
            if (m_dragIndex >= 0 && m_onAction) {
                static const WidgetPaletteAction actions[] = {
                    WidgetPaletteAction::ADD_BUTTON,
                    WidgetPaletteAction::ADD_LABEL,
                    WidgetPaletteAction::ADD_IMAGE,
                    WidgetPaletteAction::ADD_PANEL,
                    WidgetPaletteAction::ADD_LAYOUT
                };
                m_onAction(actions[m_dragIndex], event.screenPos.x, event.screenPos.y);
                m_dragIndex = -1;
            }
            return true;
        }
        default:
            return false;
    }
}
