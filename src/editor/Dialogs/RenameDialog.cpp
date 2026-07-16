#include "RenameDialog.h"
#include <Render/Renderer.h>
#include <TextRenderer.h>

RenameDialog::RenameDialog() {
    m_rectWidth = 1280.0f;
    m_rectHeight = 720.0f;
}

void RenameDialog::Show(const std::string& currentName) {
    m_visible = true;
    m_nameBox.SetValue(currentName);
    m_fieldActive = false;
    m_nameBox.Deactivate();
}

void RenameDialog::Hide() {
    m_visible = false;
    m_nameBox.Deactivate();
    m_fieldActive = false;
}

void RenameDialog::OnOK(std::function<void()> cb) {
    m_onOK = std::move(cb);
}

void RenameDialog::OnCancel(std::function<void()> cb) {
    m_onCancel = std::move(cb);
}

HitRect RenameDialog::GetHitRect() const {
    if (!m_visible) return { 0.0f, 0.0f, 0.0f, 0.0f };
    return { m_rectLeft, m_rectTop, m_rectWidth, m_rectHeight };
}

void RenameDialog::OnUpdate(float deltaTime) {
    if (!m_visible) return;
    if (m_fieldActive) {
        m_nameBox.OnUpdate(deltaTime);
    }
}

void RenameDialog::OnRender(Renderer& renderer) {
    if (!m_visible || !m_fontRenderer) return;

    float viewW = m_rectWidth;
    float viewH = m_rectHeight;

    Color overlay(0.0f, 0.0f, 0.0f, 0.5f);
    renderer.DrawQuad(Mat4::Translate(Vec3(viewW * 0.5f, viewH * 0.5f, 0.0f)),
                      Vec2(viewW * 2.0f, viewH * 2.0f), overlay);

    float dlgX = (viewW - DIALOG_W) * 0.5f;
    float dlgY = (viewH - DIALOG_H) * 0.5f;

    Color dlgBg(0.15f, 0.15f, 0.17f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + DIALOG_W * 0.5f, dlgY + DIALOG_H * 0.5f, 0.0f)),
                      Vec2(DIALOG_W, DIALOG_H), dlgBg);

    float textColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
    float base = m_fontRenderer->GetBaselineOffset(1.2f);
    m_fontRenderer->RenderString(renderer, "Rename",
        dlgX + 12.0f, dlgY + 10.0f + base, 1.2f, textColor, TextRenderer::Align::Left);

    if (m_fontRenderer) {
        float labelColor[4] = { 0.7f, 0.7f, 0.7f, 1.0f };
        m_fontRenderer->RenderString(renderer, "Name:",
            dlgX + 12.0f, dlgY + 50.0f + (FIELD_H - m_fontRenderer->GetLineHeight(1.0f)) * 0.5f + m_fontRenderer->GetBaselineOffset(1.0f),
            1.0f, labelColor, TextRenderer::Align::Left);
    }

    m_nameBox.Draw(renderer, m_fontRenderer, dlgX + 12.0f + LABEL_W, dlgY + 50.0f,
                   DIALOG_W - 24.0f - LABEL_W, FIELD_H, m_fieldActive);

    float btnY = dlgY + DIALOG_H - 40.0f;

    Color okColor(0.2f, 0.5f, 0.3f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + DIALOG_W - BTN_W * 2.0f - 16.0f + BTN_W * 0.5f, btnY + BTN_H * 0.5f, 0.0f)),
                      Vec2(BTN_W, BTN_H), okColor);
    m_fontRenderer->RenderString(renderer, "OK",
        dlgX + DIALOG_W - BTN_W * 2.0f - 16.0f, btnY + (BTN_H - m_fontRenderer->GetLineHeight(1.0f)) * 0.5f + m_fontRenderer->GetBaselineOffset(1.0f),
        1.0f, textColor, TextRenderer::Align::Center);

    Color cancelColor(0.4f, 0.2f, 0.2f, 1.0f);
    renderer.DrawQuad(Mat4::Translate(Vec3(dlgX + DIALOG_W - BTN_W - 8.0f + BTN_W * 0.5f, btnY + BTN_H * 0.5f, 0.0f)),
                      Vec2(BTN_W, BTN_H), cancelColor);
    m_fontRenderer->RenderString(renderer, "Cancel",
        dlgX + DIALOG_W - BTN_W - 8.0f, btnY + (BTN_H - m_fontRenderer->GetLineHeight(1.0f)) * 0.5f + m_fontRenderer->GetBaselineOffset(1.0f),
        1.0f, textColor, TextRenderer::Align::Center);
}

bool RenameDialog::OnMouseEvent(const MouseEvent& event) {
    if (!m_visible) return false;

    if (event.type == MouseEvent::Press && event.button == MouseEvent::Left) {
        float viewW = m_rectWidth;
        float viewH = m_rectHeight;
        float dlgX = (viewW - DIALOG_W) * 0.5f;
        float dlgY = (viewH - DIALOG_H) * 0.5f;

        float localX = event.screenPos.x;
        float localY = event.screenPos.y;

        float btnY = dlgY + DIALOG_H - 40.0f;

        // OK
        float okX = dlgX + DIALOG_W - BTN_W * 2.0f - 16.0f;
        if (localX >= okX && localX < okX + BTN_W &&
            localY >= btnY && localY < btnY + BTN_H) {
            if (m_onOK) m_onOK();
            return true;
        }

        // Cancel
        float cancelX = dlgX + DIALOG_W - BTN_W - 8.0f;
        if (localX >= cancelX && localX < cancelX + BTN_W &&
            localY >= btnY && localY < btnY + BTN_H) {
            if (m_onCancel) m_onCancel();
            return true;
        }

        // Name field
        float fieldY = dlgY + 50.0f;
        if (localY >= fieldY && localY < fieldY + FIELD_H &&
            localX >= dlgX + 12.0f + LABEL_W && localX < dlgX + 12.0f + LABEL_W + 240.0f) {
            m_fieldActive = true;
            m_nameBox.Activate(m_nameBox.GetValue());
            return true;
        }

        m_fieldActive = false;
        m_nameBox.Deactivate();

        if (localX >= dlgX && localX < dlgX + DIALOG_W &&
            localY >= dlgY && localY < dlgY + DIALOG_H) {
            return true;
        }
        return true;
    }

    return true;
}
