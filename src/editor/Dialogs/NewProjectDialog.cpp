#include "NewProjectDialog.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

NewProjectDialog::NewProjectDialog() {
    m_rectWidth = 1280.0f;
    m_rectHeight = 720.0f;
}

void NewProjectDialog::Show() {
    m_visible = true;
    m_result = Result::None;
    m_activeField = -1;
    m_nameBox.PresetValue("NewProject");
    m_pathBox.Deactivate();
}

void NewProjectDialog::Hide() {
    m_visible = false;
    m_nameBox.Deactivate();
    m_pathBox.Deactivate();
    m_activeField = -1;
}

void NewProjectDialog::SetProjectPath(const std::string& path) {
    m_pathBox.PresetValue(path);
}

void NewProjectDialog::OnOK(std::function<void()> cb) {
    m_onOK = std::move(cb);
}

void NewProjectDialog::OnBrowse(std::function<void()> cb) {
    m_onBrowse = std::move(cb);
}

HitRect NewProjectDialog::GetHitRect() const {
    if (!m_visible) return { 0.0f, 0.0f, 0.0f, 0.0f };
    return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
}

void NewProjectDialog::OnUpdate(float deltaTime) {
    if (!m_visible) return;

    if (m_activeField == 0) {
        m_nameBox.OnUpdate(deltaTime);
    } else if (m_activeField == 1) {
        m_pathBox.OnUpdate(deltaTime);
    }
}

void NewProjectDialog::DrawField(Renderer& renderer, float x, float y, float w,
                                   TextEditBox& box, const char* label) {
    if (m_fontRenderer) {
        float textColor[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
        float textH = m_fontRenderer->GetLineHeight(1.0f);
        float base = m_fontRenderer->GetBaselineOffset(1.0f);
        m_fontRenderer->RenderString(renderer, label,
            x, y + (FIELD_H - textH) * 0.5f + base,
            1.0f, textColor, TextRenderer::Align::Left);
    }

    bool isActive = (m_activeField >= 0) && (&box == &m_nameBox ? m_activeField == 0 : m_activeField == 1);
    box.Draw(renderer, m_fontRenderer, x + LABEL_W, y, w - LABEL_W - 60.0f, FIELD_H, isActive);
}

void NewProjectDialog::OnRender(Renderer& renderer) {
    if (!m_visible || !m_fontRenderer) return;

    float viewW = m_rectWidth;
    float viewH = m_rectHeight;

    Color overlay(0.0f, 0.0f, 0.0f, 0.5f);
    renderer.DrawQuad(Mat4::Translate(Vec3(viewW * 0.5f, viewH * 0.5f, 0.0f)),
                      Vec2(viewW * 2.0f, viewH * 2.0f), overlay);

    float dlgW = DIALOG_W;
    float dlgH = DIALOG_H;
    float dlgX = (viewW - dlgW) * 0.5f;
    float dlgY = (viewH - dlgH) * 0.5f;

    Color dlgBg(0.15f, 0.15f, 0.17f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + dlgW * 0.5f, dlgY + dlgH * 0.5f, 0.0f)),
                      Vec2(dlgW, dlgH), dlgBg);

    float textColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float base = m_fontRenderer->GetBaselineOffset(1.2f);
    m_fontRenderer->RenderString(renderer, "New Project",
        dlgX + 12.0f, dlgY + 10.0f + base, 1.2f, textColor, TextRenderer::Align::Left);

    float fieldY = dlgY + 50.0f;
    DrawField(renderer, dlgX + 12.0f, fieldY, dlgW - 24.0f, m_nameBox, "Name:");

    float fieldY2 = fieldY + FIELD_H + 10.0f;
    DrawField(renderer, dlgX + 12.0f, fieldY2, dlgW - 24.0f, m_pathBox, "Path:");

    // Browse button next to path field
    float browseX = dlgX + 12.0f + LABEL_W + (dlgW - 24.0f - LABEL_W - 60.0f) + 8.0f;
    Color browseColor(0.2f, 0.2f, 0.25f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(browseX + 25.0f, fieldY2 + FIELD_H * 0.5f, 0.0f)),
                      Vec2(50.0f, FIELD_H - 2.0f), browseColor);
    m_fontRenderer->RenderStringInRect(renderer, "...",
        browseX, fieldY2, 50.0f, FIELD_H - 2.0f,
        1.0f, textColor,
        TextRenderer::Align::Center, TextRenderer::VAlign::Middle);

    float btnY = dlgY + dlgH - 40.0f;

    // OK button
    Color okColor(0.2f, 0.5f, 0.3f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + dlgW - BTN_W * 2.0f - 16.0f + BTN_W * 0.5f, btnY + BTN_H * 0.5f, 0.0f)),
                      Vec2(BTN_W, BTN_H), okColor);
    m_fontRenderer->RenderStringInRect(renderer, "OK",
        dlgX + dlgW - BTN_W * 2.0f - 16.0f, btnY, BTN_W, BTN_H,
        1.0f, textColor,
        TextRenderer::Align::Center, TextRenderer::VAlign::Middle);

    // Cancel button
    Color cancelColor(0.4f, 0.2f, 0.2f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + dlgW - BTN_W - 8.0f + BTN_W * 0.5f, btnY + BTN_H * 0.5f, 0.0f)),
                      Vec2(BTN_W, BTN_H), cancelColor);
    m_fontRenderer->RenderStringInRect(renderer, "Cancel",
        dlgX + dlgW - BTN_W - 8.0f, btnY, BTN_W, BTN_H,
        1.0f, textColor,
        TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
}

bool NewProjectDialog::OnMouseEvent(const MouseEvent& event) {
    if (!m_visible) return false;

    if (event.type == MouseEvent::Press && event.button == MouseEvent::Left) {
        float viewW = m_rectWidth;
        float viewH = m_rectHeight;
        float dlgW = DIALOG_W;
        float dlgH = DIALOG_H;
        float dlgX = (viewW - dlgW) * 0.5f;
        float dlgY = (viewH - dlgH) * 0.5f;

        float localX = event.screenPos.x;
        float localY = event.screenPos.y;

        // Check browse button
        float fieldY2 = dlgY + 50.0f + FIELD_H + 10.0f;
        float browseX = dlgX + 12.0f + LABEL_W + (dlgW - 24.0f - LABEL_W - 60.0f) + 8.0f;
        if (localX >= browseX && localX < browseX + 50.0f &&
            localY >= fieldY2 && localY < fieldY2 + FIELD_H) {
            if (m_onBrowse) m_onBrowse();
            return true;
        }

        float btnY = dlgY + dlgH - 40.0f;

        // Check OK button
        float okX = dlgX + dlgW - BTN_W * 2.0f - 16.0f;
        if (localX >= okX && localX < okX + BTN_W &&
            localY >= btnY && localY < btnY + BTN_H) {
            m_result = Result::OK;
            if (m_onOK) m_onOK();
            return true;
        }

        // Check Cancel button
        float cancelX = dlgX + dlgW - BTN_W - 8.0f;
        if (localX >= cancelX && localX < cancelX + BTN_W &&
            localY >= btnY && localY < btnY + BTN_H) {
            m_result = Result::Cancel;
            Hide();
            return true;
        }

        // Check name field
        float nameFieldY = dlgY + 50.0f;
        float valueLeft = dlgX + 12.0f + LABEL_W;
        if (localY >= nameFieldY && localY < nameFieldY + FIELD_H &&
            localX >= valueLeft && localX < valueLeft + 240.0f) {
            if (m_activeField == 0) {
                m_nameBox.OnMouseDown(m_fontRenderer, localX, valueLeft, 6.0f);
            } else {
                m_pathBox.Deactivate();
                m_activeField = 0;
                m_nameBox.Activate(m_nameBox.GetValue());
            }
            return true;
        }

        // Check path field
        if (localY >= fieldY2 && localY < fieldY2 + FIELD_H &&
            localX >= valueLeft && localX < valueLeft + 240.0f) {
            if (m_activeField == 1) {
                m_pathBox.OnMouseDown(m_fontRenderer, localX, valueLeft, 6.0f);
            } else {
                m_nameBox.Deactivate();
                m_activeField = 1;
                m_pathBox.Activate(m_pathBox.GetValue());
            }
            return true;
        }

        m_nameBox.Deactivate();
        m_pathBox.Deactivate();
        m_activeField = -1;

        if (localX >= dlgX && localX < dlgX + dlgW &&
            localY >= dlgY && localY < dlgY + dlgH) {
            return true;
        }
        return true;
    }

    return true;
}
