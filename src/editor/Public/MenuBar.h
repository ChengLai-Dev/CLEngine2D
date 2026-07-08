#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>

class Renderer;
class OrthographicCamera;

enum class MenuBarAction {
    FILE_NEW, FILE_OPEN, FILE_SAVE,
    EDIT_UNDO, EDIT_REDO, EDIT_DELETE
};

class MenuBar : public IEditorPanel {
public:
    MenuBar();
    ~MenuBar();

    void SetRect(float x, float y, float width, float height) override;
    void SetWindowHeight(int height) override;

    void OnRender(Renderer& renderer) override;

    bool OnMouseEvent(const MouseEvent& event) override;
    HitRect GetHitRect() const override;

    using ActionCallback = std::function<void(MenuBarAction)>;
    void OnAction(ActionCallback cb);

private:
    void DrawDropdown(Renderer& renderer, float dropX, float dropY,
                      const char* const* items, const MenuBarAction* actions, int count);
    bool HandleClick(float mx, float my);

    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectWidth = 1280.0f;
    float m_rectHeight = 24.0f;
    int m_windowHeight = 0;
    int m_openMenu = -1;

    ActionCallback m_onAction;
    std::unique_ptr<OrthographicCamera> m_camera;
};
