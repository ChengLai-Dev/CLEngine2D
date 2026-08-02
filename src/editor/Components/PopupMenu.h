#pragma once

#include "IEditorPanel.h"
#include <Math/Vec2.h>
#include <functional>
#include <string>
#include <vector>

class PopupMenu : public IEditorPanel {
public:
    struct Item {
        std::string label;
        int data = 0;
    };

    void Open(Vec2 pos, std::vector<Item> items,
              std::function<void(int)> onSelected = nullptr,
              std::function<void()> onDismissed = nullptr);
    void Close();
    bool IsOpen() const;

    // IEditorPanel
    bool IsVisible() const override;
    void OnUpdate(float deltaTime) override;
    void OnRender(Renderer& renderer) override;
    bool OnMouseEvent(const MouseEvent& event) override;

    float itemWidth = 160.0f;
    float itemHeight = 22.0f;

private:
    int HitTest(Vec2 pos) const;

    bool m_open = false;
    Vec2 m_pos;
    std::vector<Item> m_items;
    std::function<void(int)> m_onSelected;
    std::function<void()> m_onDismissed;
};
