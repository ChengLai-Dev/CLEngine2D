#pragma once

#include "IEditorPanel.h"
#include "TextEditBox.h"
#include <functional>
#include <string>

class RenameDialog : public IEditorPanel {
public:
    RenameDialog();

    void Show(const std::string& currentName);
    void Hide();
    bool IsVisible() const { return m_visible; }

    std::string GetNewName() const { return m_nameBox.GetValue(); }

    void OnOK(std::function<void()> cb);
    void OnCancel(std::function<void()> cb);

    // IEditorPanel
    HitRect GetHitRect() const override;
    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;
    bool IsCapturing() const override { return m_visible; }

private:
    static constexpr float DIALOG_W = 380.0f;
    static constexpr float DIALOG_H = 160.0f;
    static constexpr float FIELD_H = 26.0f;
    static constexpr float LABEL_W = 60.0f;
    static constexpr float BTN_W = 80.0f;
    static constexpr float BTN_H = 28.0f;

    bool m_visible = false;
    TextEditBox m_nameBox;
    bool m_fieldActive = false;

    std::function<void()> m_onOK;
    std::function<void()> m_onCancel;
};
