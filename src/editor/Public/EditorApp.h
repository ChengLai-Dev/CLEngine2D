#pragma once

#include "EditorUISystem.h"
#include <Application.h>
#include <Math/Vec2.h>
#include <Math/Vec3.h>
#include <Scene.h>
#include <string>
#include <memory>

class Renderer;
class OrthographicCamera;

class CanvasView;
class MenuBar;
class WidgetPalette;
class PropertyPanel;
class WidgetTreePanel;

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
    void RecalculateLayout(int windowWidth, int windowHeight);
    void SelectNode(Node* node);
    void AddWidgetToScene(const std::string& type, const Vec3& position);
    void DeleteSelected();
    void SaveScene();
    void LoadScene();

    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<OrthographicCamera> m_editorCamera;

    std::unique_ptr<Scene> m_editedScene;

    std::unique_ptr<CanvasView> m_canvasView;
    std::unique_ptr<MenuBar> m_menuBar;
    std::unique_ptr<WidgetPalette> m_widgetPalette;
    std::unique_ptr<PropertyPanel> m_propertyPanel;
    std::unique_ptr<WidgetTreePanel> m_widgetTreePanel;

    EditorUISystem m_uiSystem;

    Node* m_selectedNode = nullptr;
};
