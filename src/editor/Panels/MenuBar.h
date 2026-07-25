#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>

class Renderer;

enum class MenuBarAction {
    FILE_NEW_PROJECT, FILE_OPEN_PROJECT,
    FILE_NEW_CUI_FILE, FILE_OPEN_CUI_FILE, FILE_IMPORT_CUI_FILE,
    FILE_SAVE, FILE_EXIT,
    EDIT_UNDO, EDIT_REDO, EDIT_DELETE
};

class MenuBar : public IEditorPanel {
public:
    MenuBar();
    ~MenuBar();

    void OnRender(Renderer& renderer) override;

    bool OnMouseEvent(const MouseEvent& event) override;
    HitRect GetHitRect() const override;

    using ActionCallback = std::function<void(MenuBarAction)>;
    void OnAction(ActionCallback cb);

private:
    void DrawDropdown(Renderer& renderer, float dropX, float dropY,
                      const char* const* items, const MenuBarAction* actions, int count);
    bool HandleClick(float mx, float my);

    int m_openMenu = -1;

    ActionCallback m_onAction;
};
