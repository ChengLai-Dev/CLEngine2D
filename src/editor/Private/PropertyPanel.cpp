#include "PropertyPanel.h"
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <Render/Renderer.h>

PropertyPanel::PropertyPanel() {
    m_rectWidth = 300.0f;
    m_rectHeight = 720.0f;
}

PropertyPanel::~PropertyPanel() = default;

void PropertyPanel::SetTarget(Node* target) {
    m_target = target;
}

Node* PropertyPanel::GetTarget() const {
    return m_target;
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
    Color bgColor(0.12f, 0.12f, 0.14f, 1.0f);

    Mat4 bgTransform = Mat4::Translate(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f));
    renderer.DrawQuad(bgTransform, Vec2(m_rectWidth, m_rectHeight), bgColor);

    if (m_target) {
        float y = 20.0f;

        Color labelColor(0.9f, 0.6f, 0.2f, 1.0f);

        Vec3 pos = m_target->GetPosition();
        Vec2 size = m_target->GetContentSize();

        y += 24.0f;

        Mat4 labelBg = Mat4::Translate(Vec3(10.0f + 40.0f, y, 0.0f));
        renderer.DrawQuad(labelBg, Vec2(80.0f, 20.0f), labelColor);
    }
}
