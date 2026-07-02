#include "Toolbar.h"
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>

Toolbar::Toolbar() {
    m_camera = std::make_unique<OrthographicCamera>(0.0f, 1280.0f, 36.0f, 0.0f);
}

Toolbar::~Toolbar() = default;

void Toolbar::SetRect(float x, float y, float w, float h) {
    m_rectX = x;
    m_rectY = y;
    m_rectW = w;
    m_rectH = h;
}

void Toolbar::OnAction(ActionCallback cb) {
    m_onAction = std::move(cb);
}

void Toolbar::OnRender(Renderer& renderer) {
    float bgColor[4] = { 0.08f, 0.08f, 0.1f, 1.0f };
    float btnColor[4] = { 0.2f, 0.25f, 0.3f, 1.0f };
    float sepColor[4] = { 0.3f, 0.3f, 0.35f, 1.0f };

    RenderCommand::SetViewport(
        static_cast<int>(m_rectX),
        static_cast<int>(m_rectY),
        static_cast<int>(m_rectW),
        static_cast<int>(m_rectH)
    );

    renderer.BeginScene(*m_camera);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectW * 0.5f, m_rectH * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectW, m_rectH),
                      nullptr, bgColor,
                      0.0f, 0.0f, 1.0f, 1.0f);

    float bx = 6.0f;
    float btnW = 70.0f;
    float btnH = m_rectH - 8.0f;

    ToolbarAction addActions[5] = {
        ToolbarAction::ADD_BUTTON,
        ToolbarAction::ADD_LABEL,
        ToolbarAction::ADD_IMAGE,
        ToolbarAction::ADD_PANEL,
        ToolbarAction::ADD_LAYOUT
    };

    for (int i = 0; i < 5; ++i) {
        Mat4 btnTransform = Mat4::Translate(Vec3(bx + btnW * 0.5f, m_rectH * 0.5f, 0.0f));
        renderer.DrawQuad(btnTransform, Vec2(btnW - 2.0f, btnH),
                          nullptr, btnColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
        bx += btnW;
    }

    bx += 8.0f;
    Mat4 sep1 = Mat4::Translate(Vec3(bx, m_rectH * 0.5f, 0.0f));
    renderer.DrawQuad(sep1, Vec2(2.0f, btnH * 0.6f),
                      nullptr, sepColor,
                      0.0f, 0.0f, 1.0f, 1.0f);
    bx += 10.0f;

    ToolbarAction fileActions[2] = {
        ToolbarAction::ACTION_SAVE,
        ToolbarAction::ACTION_LOAD
    };

    for (int i = 0; i < 2; ++i) {
        Mat4 btnTransform = Mat4::Translate(Vec3(bx + btnW * 0.5f, m_rectH * 0.5f, 0.0f));
        renderer.DrawQuad(btnTransform, Vec2(btnW - 2.0f, btnH),
                          nullptr, btnColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
        bx += btnW;
    }

    bx += 8.0f;
    Mat4 sep2 = Mat4::Translate(Vec3(bx, m_rectH * 0.5f, 0.0f));
    renderer.DrawQuad(sep2, Vec2(2.0f, btnH * 0.6f),
                      nullptr, sepColor,
                      0.0f, 0.0f, 1.0f, 1.0f);
    bx += 10.0f;

    ToolbarAction editActions[3] = {
        ToolbarAction::ACTION_UNDO,
        ToolbarAction::ACTION_REDO,
        ToolbarAction::ACTION_DELETE
    };

    for (int i = 0; i < 3; ++i) {
        Mat4 btnTransform = Mat4::Translate(Vec3(bx + btnW * 0.5f, m_rectH * 0.5f, 0.0f));
        renderer.DrawQuad(btnTransform, Vec2(btnW - 2.0f, btnH),
                          nullptr, btnColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
        bx += btnW;
    }

    renderer.EndScene();

    RenderCommand::SetViewport(0, 0, 1280, 720);
}
