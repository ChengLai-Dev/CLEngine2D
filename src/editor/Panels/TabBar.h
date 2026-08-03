#pragma once

#include "IEditorPanel.h"
#include <string>
#include <vector>
#include <functional>

class TabBar : public IEditorPanel {
public:
    struct TabInfo {
        std::string name;
        bool dirty = false;
        bool external = false;
        std::string filePath;
    };

    TabBar();

    void SetTabs(const std::vector<TabInfo>& tabs);
    void SetActiveTab(int index);
    int GetActiveTab() const { return m_activeTab; }

    // IEditorPanel
    HitRect GetHitRect() const override;
    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;

    using TabSwitchCallback = std::function<void(int index)>;
    using TabCloseCallback = std::function<void(int index)>;
    void OnTabSwitch(TabSwitchCallback cb);
    void OnTabClose(TabCloseCallback cb);

    static constexpr float TAB_BAR_HEIGHT = 28.0f;
    static constexpr float CLOSE_BTN_SIZE = 14.0f;

private:
    float GetTabStartX() const { return 4.0f; }
    float GetTabWidth() const { return 130.0f; }

    int HitTestTab(float mx) const;
    int HitTestClose(float mx, float my) const;

    static constexpr float TOOLTIP_DELAY = 0.7f;
    static constexpr float MOVE_THRESHOLD = 3.0f;
    static constexpr float TAB_TEXT_LEFT = 8.0f;
    static constexpr float MAX_TEXT_WIDTH = 100.0f;
    static constexpr float TOOLTIP_EXTRA_HEIGHT = 60.0f;
    static constexpr float TOOLTIP_OFFSET = 12.0f;

    std::vector<TabInfo> m_tabs;
    int m_activeTab = -1;

    int m_hoveredTab = -1;
    int m_hoveredClose = -1;
    float m_hoverTimer = 0.0f;
    float m_mouseX = 0.0f;
    float m_mouseY = 0.0f;
    float m_lastHoverX = 0.0f;
    float m_lastHoverY = 0.0f;
    int m_tooltipTabIndex = -1;

    TabSwitchCallback m_onTabSwitch;
    TabCloseCallback m_onTabClose;
};
