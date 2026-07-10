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

    using SelectionChangedCallback = std::function<void(Node*)>;
    void OnSelectionChanged(SelectionChangedCallback cb);

    Node* GetSelectedNode() const;

private:
    void VisitNode(Node* node, Renderer& renderer, float& y, int depth);

    Node* m_root = nullptr;
    Node* m_selectedNode = nullptr;

    SelectionChangedCallback m_onSelectionChanged;
};
