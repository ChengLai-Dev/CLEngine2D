#pragma once

#include "IEditorPanel.h"
#include "PopupMenu.h"
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
    void OnUpdate(float deltaTime) override;

    using ActionCallback = std::function<void(MenuBarAction)>;
    void OnAction(ActionCallback cb);

    PopupMenu* GetPopupMenu() { return &m_popupMenu; }

private:
    bool HandleClick(float mx, float my);

    PopupMenu m_popupMenu;
    int m_openIndex = -1;
    bool m_dismissedThisFrame = false;

    ActionCallback m_onAction;
};
