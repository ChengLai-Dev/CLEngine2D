#pragma once

#include "IEditorPanel.h"
#include "TextEditBox.h"
#include <functional>
#include <string>

class NewCuiFileDialog : public IEditorPanel {
public:
    enum class Result { None, OK, Cancel };

    NewCuiFileDialog();

    void Show();
    void Hide();
    bool IsVisible() const override { return m_visible; }
    Result GetResult() const { return m_result; }

    std::string GetFileName() const { return m_nameBox.GetValue(); }
    std::string GetDirectory() const { return m_dirBox.GetValue(); }

    void SetFileName(const std::string& name);
    void SetDirectory(const std::string& dir);

    void OnOK(std::function<void()> cb);
    void OnBrowse(std::function<void()> cb);

    // IEditorPanel
    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;
    bool IsCapturing() const override { return m_visible; }

private:
    static constexpr float DIALOG_W = 420.0f;
    static constexpr float DIALOG_H = 200.0f;
    static constexpr float FIELD_H = 26.0f;
    static constexpr float LABEL_W = 80.0f;
    static constexpr float BTN_W = 80.0f;
    static constexpr float BTN_H = 28.0f;

    void DrawField(Renderer& renderer, float x, float y, float w,
                   TextEditBox& box, const char* label);

    bool m_visible = false;
    Result m_result = Result::None;

    TextEditBox m_nameBox;
    TextEditBox m_dirBox;
    int m_activeField = -1;

    std::function<void()> m_onOK;
    std::function<void()> m_onBrowse;
};
