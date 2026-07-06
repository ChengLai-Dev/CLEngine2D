#include "PropertyPanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>
#include <Render/OrthographicCamera.h>
#include <Render/RenderCommand.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <glad/glad.h>

PropertyPanel::PropertyPanel() {
    m_camera = std::make_unique<OrthographicCamera>(0.0f, 100.0f, 100.0f, 0.0f);
}

PropertyPanel::~PropertyPanel() = default;

void PropertyPanel::SetTarget(Node* target) {
    m_target = target;
}

Node* PropertyPanel::GetTarget() const {
    return m_target;
}

void PropertyPanel::SetRect(float x, float y, float w, float h) {
    m_rectX = x;
    m_rectY = y;
    m_rectW = w;
    m_rectH = h;
    m_camera->SetProjection(0.0f, w, h, 0.0f);
}

void PropertyPanel::OnPropertyChanged(PropertyChangedCallback cb) {
    m_onPropertyChanged = std::move(cb);
}

void PropertyPanel::DrawProperty(const char* label, float value, float minVal, float maxVal,
                                  Renderer& renderer, float& y,
                                  std::function<void(float)> setter) {
    (void)minVal;
    (void)maxVal;
    (void)setter;

    float bgColor[4] = { 0.15f, 0.15f, 0.17f, 1.0f };
    float labelColor[4] = { 0.8f, 0.8f, 0.8f, 1.0f };

    (void)renderer;

    y += 24.0f;
}

void PropertyPanel::OnRender(Renderer& renderer) {
    float bgColor[4] = { 0.12f, 0.12f, 0.14f, 1.0f };

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

    if (m_target) {
        float y = 20.0f;

        float labelColor[4] = { 0.9f, 0.6f, 0.2f, 1.0f };
        (void)labelColor;

        Vec3 pos = m_target->GetPosition();
        Vec2 size = m_target->GetContentSize();

        y += 24.0f;

        Mat4 labelBg = Mat4::Translate(Vec3(10.0f + 40.0f, y, 0.0f));
        renderer.DrawQuad(labelBg, Vec2(80.0f, 20.0f),
                          nullptr, labelColor,
                          0.0f, 0.0f, 1.0f, 1.0f);
    }

    renderer.EndScene();
}
