#pragma once

#include "EditorUISystem.h"
#include "EditorLayoutConfig.h"
#include "EditorState.h"
#include "Project.h"
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
class ResourcePanel;
class NewProjectDialog;
class NewCuiFileDialog;
class RenameDialog;

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
        bool external = false;
    };

    void RenderPanel(IEditorPanel* panel, bool useCustomProj = false);
    void RecalculateLayout(int windowWidth, int windowHeight);
    void SelectWidget(Widget* widget);
    void AddWidgetToScene(const std::string& type, const Vec3& position, Node* parentOverride = nullptr);
    void DeleteSelected();
    void DrawPanelBorders();

    void SaveLayout();
    void LoadLayout();
    void SaveEditorState();
    void LoadEditorState();
    void SaveProjectFile();

    void NewTab();
    void SwitchTab(int index);
    void CloseTab(int index);
    void CloseAllTabs();
    void SaveActiveTab();
    void OpenCuiFile(const std::string& filepath);
    void LoadCuiFileIntoNewTab(const std::string& filepath, bool external = false);
    void SetTabDirty();
    bool HasUnsavedTabs() const;
    bool IsFileOpenInTab(const std::string& filepath) const;
    void SyncTabToPanels();
    void UpdateTabBar();

    // Project management
    void CreateNewProject(const std::string& name, const std::string& directory);
    void OpenProject(const std::string& projectFilePath);
    void CloseProject();
    void CreateNewCuiFile(const std::string& filename);
    void DeleteCuiFile(const std::string& filename);
    void RenameCuiFile(const std::string& oldName, const std::string& newName);
    void ImportCuiFile();
    void UpdateResourcePanel();
    void HandleCuiFileClick(const std::string& filename);

    // Dialog handlers
    void ShowNewProjectDialog();
    void ShowNewCuiFileDialog();
    void ShowRenameDialog(const std::string& oldName);
    void ShowWelcomeOrRestore();

    std::unique_ptr<Renderer> m_renderer;

    std::vector<EditorTab> m_tabs;
    int m_activeTabIndex = -1;

    Project m_project;
    bool m_projectOpen = false;
    EditorState m_editorState;

    std::unique_ptr<CanvasView> m_canvasView;
    std::unique_ptr<MenuBar> m_menuBar;
    std::unique_ptr<WidgetPalette> m_widgetPalette;
    std::unique_ptr<PropertyPanel> m_propertyPanel;
    std::unique_ptr<WidgetTreePanel> m_widgetTreePanel;
    std::unique_ptr<ResourcePanel> m_resourcePanel;

    EditorUISystem m_uiSystem;
    EditorLayoutConfig m_layoutConfig;

    Widget* m_selectedWidget = nullptr;

    // Panel dividers
    std::unique_ptr<PanelDivider> m_leftDivider;
    std::unique_ptr<PanelDivider> m_rightDivider;
    std::unique_ptr<PanelDivider> m_leftDivider1;
    std::unique_ptr<PanelDivider> m_leftDivider2;
    std::unique_ptr<TabBar> m_tabBar;

    // Dialogs
    std::unique_ptr<NewProjectDialog> m_newProjectDialog;
    std::unique_ptr<NewCuiFileDialog> m_newCuiFileDialog;
    std::unique_ptr<RenameDialog> m_renameDialog;

    std::unique_ptr<TextRenderer> m_fontRenderer;

    std::string m_pendingRenameFile;
};
