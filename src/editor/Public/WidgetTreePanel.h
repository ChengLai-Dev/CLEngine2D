#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>
#include <vector>

class Node;
class Widget;
class Renderer;

class WidgetTreePanel : public IEditorPanel {
public:
    WidgetTreePanel();
    ~WidgetTreePanel();

    void SetRoot(Node* root);
    void OnRender(Renderer& renderer) override;

    bool OnMouseEvent(const MouseEvent& event) override;

    void SelectNode(Node* node);

    using SelectionChangedCallback = std::function<void(Node*)>;
    void OnSelectionChanged(SelectionChangedCallback cb);

    Node* GetSelectedNode() const;

private:
    void DrawWidgetTree(Node* node, Renderer& renderer, float& y, int depth);
    Node* HitTest(Node* node, float& y, float my) const;

    Node* m_root = nullptr;
    Node* m_selectedNode = nullptr;

    SelectionChangedCallback m_onSelectionChanged;
};
