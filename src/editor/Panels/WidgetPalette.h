#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>

class Renderer;

enum class WidgetPaletteAction {
    ADD_BUTTON, ADD_LABEL, ADD_IMAGE, ADD_PANEL, ADD_LAYOUT
};

class WidgetPalette : public IEditorPanel {
public:
    WidgetPalette();
    ~WidgetPalette();

    void OnRender(Renderer& renderer) override;

    bool OnMouseEvent(const MouseEvent& event) override;
    bool IsCapturing() const override { return m_dragIndex >= 0; }

    using ActionCallback = std::function<void(WidgetPaletteAction, float mouseX, float mouseY)>;
    void OnAction(ActionCallback cb);

    float CalcDesiredHeight(float panelWidth) const;

private:
    static constexpr float kPadding = 5.0f;
    static constexpr float kGap = 4.0f;
    static constexpr float kBtnHeight = 28.0f;
    static constexpr float kMinBtnWidth = 60.0f;
    static constexpr int kBtnCount = 5;

    int CalcButtonsPerRow(float panelWidth) const;
    int CalcRowCount(float panelWidth) const;
    HitRect GetButtonRect(int index) const;

    int m_dragIndex = -1;

    ActionCallback m_onAction;
};
