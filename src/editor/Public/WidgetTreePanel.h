#pragma once

#include "IEditorPanel.h"
#include <memory>
#include <functional>
#include <vector>

class Node;
class Widget;
class Renderer;
class OrthographicCamera;

class WidgetTreePanel : public IEditorPanel {
public:
    WidgetTreePanel();
    ~WidgetTreePanel();

    void SetRoot(Node* root);
    void SetRect(float x, float y, float width, float height) override;
    void SetWindowHeight(int height) override;

    void OnRender(Renderer& renderer) override;

    HitRect GetHitRect() const override;

    using SelectionChangedCallback = std::function<void(Node*)>;
    void OnSelectionChanged(SelectionChangedCallback cb);

    Node* GetSelectedNode() const;

private:
    void VisitNode(Node* node, Renderer& renderer, float& y, int depth);

    Node* m_root = nullptr;
    Node* m_selectedNode = nullptr;

    float m_rectX = 0.0f;
    float m_rectY = 0.0f;
    float m_rectWidth = 250.0f;
    float m_rectHeight = 720.0f;

    int m_windowHeight = 0;

    SelectionChangedCallback m_onSelectionChanged;

    std::unique_ptr<OrthographicCamera> m_camera;
};
