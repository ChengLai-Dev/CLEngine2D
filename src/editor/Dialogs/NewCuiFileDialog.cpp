#include "NewCuiFileDialog.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

NewCuiFileDialog::NewCuiFileDialog() {
    m_rectWidth = 1280.0f;
    m_rectHeight = 720.0f;
}

void NewCuiFileDialog::Show() {
    m_visible = true;
    m_result = Result::None;
    m_activeField = -1;
    m_nameBox.Deactivate();
    m_dirBox.Deactivate();
}

void NewCuiFileDialog::Hide() {
    m_visible = false;
    m_nameBox.Deactivate();
    m_dirBox.Deactivate();
    m_activeField = -1;
}

void NewCuiFileDialog::SetFileName(const std::string& name) {
    m_nameBox.PresetValue(name);
}

void NewCuiFileDialog::SetDirectory(const std::string& dir) {
    m_dirBox.PresetValue(dir);
}

void NewCuiFileDialog::OnOK(std::function<void()> cb) {
    m_onOK = std::move(cb);
}

void NewCuiFileDialog::OnBrowse(std::function<void()> cb) {
    m_onBrowse = std::move(cb);
}

HitRect NewCuiFileDialog::GetHitRect() const {
    if (!m_visible) return { 0.0f, 0.0f, 0.0f, 0.0f };
    return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
}

void NewCuiFileDialog::OnUpdate(float deltaTime) {
    if (!m_visible) return;

    if (m_activeField == 0) {
        m_nameBox.OnUpdate(deltaTime);
    } else if (m_activeField == 1) {
        m_dirBox.OnUpdate(deltaTime);
    }
}

void NewCuiFileDialog::DrawField(Renderer& renderer, float x, float y, float w,
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
    box.Draw(renderer, m_fontRenderer, x + LABEL_W, y, w - LABEL_W, FIELD_H, isActive);
}

void NewCuiFileDialog::OnRender(Renderer& renderer) {
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
    m_fontRenderer->RenderString(renderer, "New CUI File",
        dlgX + 12.0f, dlgY + 10.0f + base, 1.2f, textColor, TextRenderer::Align::Left);

    float fieldY = dlgY + 50.0f;
    DrawField(renderer, dlgX + 12.0f, fieldY, dlgW - 24.0f, m_nameBox, "Name:");

    float fieldY2 = fieldY + FIELD_H + 10.0f;
    DrawField(renderer, dlgX + 12.0f, fieldY2, dlgW - 24.0f, m_dirBox, "Dir:");

    float btnY = dlgY + dlgH - 40.0f;

    // OK button
    Color okColor(0.2f, 0.5f, 0.3f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + dlgW - BTN_W * 2.0f - 16.0f + BTN_W * 0.5f, btnY + BTN_H * 0.5f, 0.0f)),
                      Vec2(BTN_W, BTN_H), okColor);
    m_fontRenderer->RenderString(renderer, "OK",
        dlgX + dlgW - BTN_W * 2.0f - 16.0f, btnY + (BTN_H - m_fontRenderer->GetLineHeight(1.0f)) * 0.5f + base,
        1.0f, textColor, TextRenderer::Align::Center);

    // Cancel button
    Color cancelColor(0.4f, 0.2f, 0.2f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + dlgW - BTN_W - 8.0f + BTN_W * 0.5f, btnY + BTN_H * 0.5f, 0.0f)),
                      Vec2(BTN_W, BTN_H), cancelColor);
    m_fontRenderer->RenderString(renderer, "Cancel",
        dlgX + dlgW - BTN_W - 8.0f, btnY + (BTN_H - m_fontRenderer->GetLineHeight(1.0f)) * 0.5f + base,
        1.0f, textColor, TextRenderer::Align::Center);
}

bool NewCuiFileDialog::OnMouseEvent(const MouseEvent& event) {
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

        float btnY = dlgY + dlgH - 40.0f;

        // OK button
        float okX = dlgX + dlgW - BTN_W * 2.0f - 16.0f;
        if (localX >= okX && localX < okX + BTN_W &&
            localY >= btnY && localY < btnY + BTN_H) {
            m_result = Result::OK;
            if (m_onOK) m_onOK();
            return true;
        }

        // Cancel button
        float cancelX = dlgX + dlgW - BTN_W - 8.0f;
        if (localX >= cancelX && localX < cancelX + BTN_W &&
            localY >= btnY && localY < btnY + BTN_H) {
            m_result = Result::Cancel;
            Hide();
            return true;
        }

        // Name field
        float nameFieldY = dlgY + 50.0f;
        if (localY >= nameFieldY && localY < nameFieldY + FIELD_H &&
            localX >= dlgX + 12.0f + LABEL_W && localX < dlgX + 12.0f + LABEL_W + 240.0f) {
            m_dirBox.Deactivate();
            m_activeField = 0;
            m_nameBox.Activate(m_nameBox.GetValue());
            return true;
        }

        // Dir field
        float dirFieldY = dlgY + 50.0f + FIELD_H + 10.0f;
        if (localY >= dirFieldY && localY < dirFieldY + FIELD_H &&
            localX >= dlgX + 12.0f + LABEL_W && localX < dlgX + 12.0f + LABEL_W + 240.0f) {
            m_nameBox.Deactivate();
            m_activeField = 1;
            m_dirBox.Activate(m_dirBox.GetValue());
            return true;
        }

        m_nameBox.Deactivate();
        m_dirBox.Deactivate();
        m_activeField = -1;

        if (localX >= dlgX && localX < dlgX + dlgW &&
            localY >= dlgY && localY < dlgY + dlgH) {
            return true;
        }
        return true;
    }

    return true;
}
