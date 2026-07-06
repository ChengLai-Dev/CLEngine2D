#pragma once

#include <Application.h>
#include <Math/Vec2.h>
#include <Scene.h>
#include <string>
#include <memory>

class Renderer;
class OrthographicCamera;

class CanvasView;
class PropertyPanel;
class WidgetTreePanel;
class Toolbar;
class Gizmo;

class EditorApp : public Application {
public:
    EditorApp();
    ~EditorApp();

protected:
    void OnInit() override;
    void OnUpdate(float deltaTime) override;
    void OnRender() override;
    void OnShutdown() override;
    void OnWindowResize(int width, int height) override;

private:
    void RecalculateLayout(int windowW, int windowH);
    void OnToolbarAction(int action);
    void SelectNode(Node* node);
    void AddWidgetToScene(const std::string& type);
    void DeleteSelected();
    void SaveScene();
    void LoadScene();

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<OrthographicCamera> m_editorCamera;

    std::unique_ptr<Scene> m_editedScene;
    std::unique_ptr<Scene> m_uiScene;

    std::unique_ptr<CanvasView> m_canvasView;
    std::unique_ptr<PropertyPanel> m_propertyPanel;
    std::unique_ptr<WidgetTreePanel> m_widgetTreePanel;
    std::unique_ptr<Toolbar> m_toolbar;

    Node* m_selectedNode = nullptr;

    bool m_isDragging = false;
    bool m_isPanning = false;
    Vec2 m_lastMousePos;
};
