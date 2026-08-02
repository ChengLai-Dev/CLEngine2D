#include "MenuBar.h"
#include "PopupMenu.h"
#include <Math/Vec2.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>

static const char* MENU_LABELS[] = { "File", "Edit" };
static const int MENU_COUNT = 2;

static const PopupMenu::Item FILE_ITEMS[] = {
    { "New Project",        static_cast<int>(MenuBarAction::FILE_NEW_PROJECT) },
    { "Open Project",       static_cast<int>(MenuBarAction::FILE_OPEN_PROJECT) },
    { "New CUI File",       static_cast<int>(MenuBarAction::FILE_NEW_CUI_FILE) },
    { "Open CUI File",      static_cast<int>(MenuBarAction::FILE_OPEN_CUI_FILE) },
    { "Import CUI File",    static_cast<int>(MenuBarAction::FILE_IMPORT_CUI_FILE) },
    { "Save",               static_cast<int>(MenuBarAction::FILE_SAVE) },
    { "Exit",               static_cast<int>(MenuBarAction::FILE_EXIT) },
};

static const PopupMenu::Item EDIT_ITEMS[] = {
    { "Undo",   static_cast<int>(MenuBarAction::EDIT_UNDO) },
    { "Redo",   static_cast<int>(MenuBarAction::EDIT_REDO) },
    { "Delete", static_cast<int>(MenuBarAction::EDIT_DELETE) },
};

MenuBar::MenuBar() {
    m_rectWidth = 1280.0f;
    m_rectHeight = 24.0f;
}

MenuBar::~MenuBar() = default;

void MenuBar::OnAction(ActionCallback cb) {
    m_onAction = std::move(cb);
}

void MenuBar::OnRender(Renderer& renderer) {
    Color bgColor(0.0f, 0.0f, 0.0f, 1.0f);
    float textColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };

    renderer.DrawQuad(Vec3(m_rectWidth * 0.5f, m_rectHeight * 0.5f, 0.0f),
                      Vec3(m_rectWidth, m_rectHeight, 1.0f), bgColor);

    float labelX = 12.0f;
    float labelW = 60.0f;
    for (int i = 0; i < MENU_COUNT; ++i) {
        renderer.DrawQuad(Vec3(labelX + labelW * 0.5f, m_rectHeight * 0.5f, 0.0f),
                          Vec3(labelW, m_rectHeight, 1.0f), bgColor);

        if (m_fontRenderer) {
            m_fontRenderer->RenderStringInRect(renderer, MENU_LABELS[i],
                labelX, 0.0f, labelW, m_rectHeight,
                1.0f, textColor,
                TextRenderer::Align::Center, TextRenderer::VAlign::Middle);
        }

        labelX += labelW;
    }
}

bool MenuBar::OnMouseEvent(const MouseEvent& event) {
    IEditorPanel::OnMouseEvent(event);
    if (event.type != MouseEvent::Press || event.button != MouseEvent::Left) return false;
    return HandleClick(event.screenPos.x, event.screenPos.y);
}

void MenuBar::OnUpdate(float deltaTime) {
    (void)deltaTime;
    m_dismissedThisFrame = false;
}

bool MenuBar::HandleClick(float mx, float my) {
    bool inBar = mx >= m_rectLeft && mx < m_rectLeft + m_rectWidth &&
                 my >= m_rectTop && my < m_rectTop + m_rectHeight;

    if (!inBar) return false;

    float labelX = 12.0f;
    float labelW = 60.0f;
    for (int i = 0; i < MENU_COUNT; ++i) {
        if (mx >= m_rectLeft + labelX && mx < m_rectLeft + labelX + labelW &&
            my >= m_rectTop && my < m_rectTop + m_rectHeight) {

            // Toggle: click the open menu's label closes it. The popup was
            // already dismissed by this very click (dismissed before the
            // event passed through), so do not reopen.
            if (m_openIndex == i) {
                if (m_dismissedThisFrame) {
                    m_openIndex = -1;
                    return true;
                }
                if (m_popupMenu.IsOpen()) {
                    m_popupMenu.Close();
                    m_openIndex = -1;
                    return true;
                }
            }

            const PopupMenu::Item* items = (i == 0) ? FILE_ITEMS : EDIT_ITEMS;
            int count = (i == 0) ? 7 : 3;
            Vec2 dropPos(m_rectLeft + ((i == 0) ? 0.0f : 60.0f), m_rectTop + m_rectHeight);

            m_popupMenu.Open(
                dropPos,
                std::vector<PopupMenu::Item>(items, items + count),
                [this](int data) {
                    m_openIndex = -1;
                    if (m_onAction) m_onAction(static_cast<MenuBarAction>(data));
                },
                [this]() {
                    m_dismissedThisFrame = true;
                });
            m_openIndex = i;
            return true;
        }
        labelX += labelW;
    }

    return false;
}
