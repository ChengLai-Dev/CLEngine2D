#pragma once

#include "EditorUISystem.h"
#include "EditorLayoutConfig.h"
#include <Application.h>
#include <Math/Vec2.h>
#include <Math/Vec3.h>
#include <Scene.h>
#include <string>
#include <memory>
#include <vector>
#include <SceneGraph/Widget.h>

class Renderer;
class TextRenderer;

class CanvasView;
class MenuBar;
class WidgetPalette;
class PropertyPanel;
class WidgetTreePanel;
class PanelDivider;
class TabBar;

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
    struct EditorTab {
        std::string name;
        std::string filePath;
        std::unique_ptr<Scene> scene;
        bool dirty = false;
    };

    void RenderPanel(IEditorPanel* panel, bool useCustomProj = false);
    void RecalculateLayout(int windowWidth, int windowHeight);
    void SelectWidget(Widget* widget);
    void AddWidgetToScene(const std::string& type, const Vec3& position);
    void DeleteSelected();
    void DrawPanelBorders();

    void SaveLayout();
    void LoadLayout();

    void NewTab();
    void SwitchTab(int index);
    void CloseTab(int index);
    void SaveActiveTab();
    void LoadSceneIntoNewTab();
    void SetTabDirty();
    bool HasUnsavedTabs() const;
    void SyncTabToPanels();
    void UpdateTabBar();

    std::unique_ptr<Renderer> m_renderer;

    std::vector<EditorTab> m_tabs;
    int m_activeTabIndex = -1;

    std::unique_ptr<CanvasView> m_canvasView;
    std::unique_ptr<MenuBar> m_menuBar;
    std::unique_ptr<WidgetPalette> m_widgetPalette;
    std::unique_ptr<PropertyPanel> m_propertyPanel;
    std::unique_ptr<WidgetTreePanel> m_widgetTreePanel;

    EditorUISystem m_uiSystem;
    EditorLayoutConfig m_layoutConfig;

    Widget* m_selectedWidget = nullptr;

    std::unique_ptr<PanelDivider> m_leftDivider;
    std::unique_ptr<PanelDivider> m_rightDivider;
    std::unique_ptr<TabBar> m_tabBar;

    std::unique_ptr<TextRenderer> m_fontRenderer;
};
