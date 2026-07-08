#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>

class Renderer;
class OrthographicCamera;

enum class WidgetPaletteAction {
    ADD_BUTTON, ADD_LABEL, ADD_IMAGE, ADD_PANEL, ADD_LAYOUT
};

class WidgetPalette : public IEditorPanel {
public:
    WidgetPalette();
    ~WidgetPalette();

    void SetRect(float x, float y, float width, float height) override;
    void SetWindowHeight(int height) override;

    void OnRender(Renderer& renderer) override;

    bool OnMouseEvent(const MouseEvent& event) override;
    HitRect GetHitRect() const override;
    bool IsCapturing() const override { return m_dragIndex >= 0; }

    using ActionCallback = std::function<void(WidgetPaletteAction, float mouseX, float mouseY)>;
    void OnAction(ActionCallback cb);

private:
    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectWidth = 250.0f;
    float m_rectHeight = 50.0f;
    int m_windowHeight = 0;
    int m_dragIndex = -1;

    ActionCallback m_onAction;
    std::unique_ptr<OrthographicCamera> m_camera;
};
