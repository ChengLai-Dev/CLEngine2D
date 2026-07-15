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
    };

    TabBar();

    void SetTabs(const std::vector<TabInfo>& tabs);
    void SetActiveTab(int index);
    int GetActiveTab() const { return m_activeTab; }

    // IEditorPanel
    HitRect GetHitRect() const override;
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

    std::vector<TabInfo> m_tabs;
    int m_activeTab = -1;

    int m_hoveredTab = -1;
    int m_hoveredClose = -1;

    TabSwitchCallback m_onTabSwitch;
    TabCloseCallback m_onTabClose;
};
