#pragma once

#include <memory>
#include <functional>
#include <vector>

class Node;
class Widget;
class Renderer;
class OrthographicCamera;

class WidgetTreePanel {
public:
    WidgetTreePanel();
    ~WidgetTreePanel();

    void SetRoot(Node* root);
    void SetRect(float x, float y, float w, float h);

    void OnRender(Renderer& renderer);

    using SelectionChangedCallback = std::function<void(Node*)>;
    void OnSelectionChanged(SelectionChangedCallback cb);

    Node* GetSelectedNode() const;

private:
    void VisitNode(Node* node, Renderer& renderer, float& y, int depth);

    Node* m_root = nullptr;
    Node* m_selectedNode = nullptr;

    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectW = 250.0f;
    float m_rectH = 720.0f;

    SelectionChangedCallback m_onSelectionChanged;

    std::unique_ptr<OrthographicCamera> m_camera;
};
