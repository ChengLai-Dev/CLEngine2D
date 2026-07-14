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
    bool IsCapturing() const override { return m_isDragging || m_isDragPending; }

    void SelectNode(Node* node);

    using SelectionChangedCallback = std::function<void(Node*)>;
    void OnSelectionChanged(SelectionChangedCallback cb);

    Node* GetSelectedNode() const;

private:
    struct NodePos {
        Node* node;
        float y;
        float depth;
    };

    void DrawWidgetTree(Node* node, Renderer& renderer, float& y, int depth);
    void DrawDropIndicator(Renderer& renderer, float y);
    Node* HitTest(Node* node, float& y, float my) const;
    void CollectPositions(Node* node, float& y, int depth, std::vector<NodePos>& out) const;

    void CommitDrag();
    void ReindexZOrder(Node* parent);

    Node* m_root = nullptr;
    Node* m_selectedNode = nullptr;

    SelectionChangedCallback m_onSelectionChanged;

    // Drag state
    bool m_isDragPending = false;
    bool m_isDragging = false;
    Node* m_dragNode = nullptr;
    Vec2 m_dragStartMouse;

    Node* m_dropTarget = nullptr;
    bool m_dropBefore = false;
};
