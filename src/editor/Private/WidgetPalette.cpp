#include "WidgetPalette.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

WidgetPalette::WidgetPalette() = default;
WidgetPalette::~WidgetPalette() = default;

void WidgetPalette::OnAction(ActionCallback cb) {
    m_onAction = std::move(cb);
}

bool WidgetPalette::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    switch (event.type) {
        case MouseEvent::Down: {
            if (event.button != MouseEvent::Left) return false;
            float localX = event.screenPos.x - m_rectLeft;
            float btnWidth = 46.0f;
            float padding = 5.0f;
            int index = static_cast<int>((localX - padding) / btnWidth);
            if (index >= 0 && index < 5) {
                m_dragIndex = index;
            }
            return m_dragIndex >= 0;
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

void WidgetPalette::OnRender(Renderer& renderer) {
    Color bgColor(0.08f, 0.08f, 0.1f, 1.0f);
    Color btnColor(0.2f, 0.25f, 0.3f, 1.0f);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight), bgColor);

    float padding = 5.0f;
    float btnWidth = 46.0f;
    float btnHeight = m_rectHeight - 8.0f;
    float bx = padding;

    static const char* btnLabels[] = { "Button", "Label", "Image", "Panel", "Layout" };

    for (int i = 0; i < 5; ++i) {
        Mat4 btnTransform = Mat4::Translate(Vec3(bx + btnWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
        renderer.DrawQuad(btnTransform, Vec2(btnWidth - 2.0f, btnHeight),
                          btnColor);

        if (m_fontRenderer) {
            float textColor[4] = { 0.85f, 0.85f, 0.85f, 1.0f };
            m_fontRenderer->RenderStringInRect(renderer, btnLabels[i],
                bx, 0.0f, btnWidth, m_rectHeight,
                1.0f, textColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }

        bx += btnWidth;
    }
}
