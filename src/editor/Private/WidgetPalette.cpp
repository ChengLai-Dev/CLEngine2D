#include "WidgetPalette.h"
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>

WidgetPalette::WidgetPalette() {
    m_camera = std::make_unique<OrthographicCamera>(0.0f, 100.0f, 100.0f, 0.0f);
}

WidgetPalette::~WidgetPalette() = default;

void WidgetPalette::SetRect(float x, float y, float width, float height) {
    m_rectLeft = x;
    m_rectTop = y;
    m_rectWidth = width;
    m_rectHeight = height;
    m_camera->SetProjection(0.0f, width, height, 0.0f);
}

void WidgetPalette::SetWindowHeight(int height) {
    m_windowHeight = height;
}

void WidgetPalette::OnAction(ActionCallback cb) {
    m_onAction = std::move(cb);
}

IEditorPanel::HitRect WidgetPalette::GetHitRect() const {
    return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
}

bool WidgetPalette::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    switch (event.type) {
        case MouseEvent::Down: {
            if (event.button != 0) return false;
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
    float bgColor[4] = { 0.08f, 0.08f, 0.1f, 1.0f };
    float btnColor[4] = { 0.2f, 0.25f, 0.3f, 1.0f };

    float vpY = static_cast<float>(m_windowHeight) - m_rectTop - m_rectHeight;

    RenderCommand::SetViewport(
        static_cast<int>(m_rectLeft),
        static_cast<int>(vpY),
        static_cast<int>(m_rectWidth),
        static_cast<int>(m_rectHeight)
    );

    renderer.BeginScene(*m_camera);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight),
                      nullptr, bgColor, 0.0f, 0.0f, 1.0f, 1.0f);

    float padding = 5.0f;
    float btnWidth = 46.0f;
    float btnHeight = m_rectHeight - 8.0f;
    float bx = padding;

    for (int i = 0; i < 5; ++i) {
        Mat4 btnTransform = Mat4::Translate(Vec3(bx + btnWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
        renderer.DrawQuad(btnTransform, Vec2(btnWidth - 2.0f, btnHeight),
                          nullptr, btnColor, 0.0f, 0.0f, 1.0f, 1.0f);
        bx += btnWidth;
    }

    renderer.EndScene();
}
