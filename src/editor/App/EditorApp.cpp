#include "EditorApp.h"
#include "EditorUISystem.h"
#include "CanvasView.h"
#include "MenuBar.h"
#include "WidgetPalette.h"
#include "PropertyPanel.h"
#include "WidgetTreePanel.h"
#include "ResourcePanel.h"
#include "Gizmo.h"
#include <UI/UISerializer.h>
#include "UndoRedo.h"
#include "TabBar.h"
#include "NewProjectDialog.h"
#include "NewCuiFileDialog.h"
#include "RenameDialog.h"
#include "FileDialog.h"

#include <Scene.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Button.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/Sprite.h>
#include <SceneGraph/Image.h>
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/Layout.h>
#include <SceneGraph/UISystem.h>
#include <Input/InputSystem.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Platform/Window.h>
#include <TextRenderer.h>
#include <Logger.h>

#include "PanelDivider.h"
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#include <objbase.h>
#include <Cursor.h>
#include <algorithm>
#include <filesystem>

static const char* GetWidgetType(WidgetPaletteAction action) {
    switch (action) {
        case WidgetPaletteAction::ADD_BUTTON: return "Button";
        case WidgetPaletteAction::ADD_LABEL:  return "Label";
        case WidgetPaletteAction::ADD_IMAGE:  return "Image";
        case WidgetPaletteAction::ADD_PANEL:  return "Panel";
        case WidgetPaletteAction::ADD_LAYOUT: return "Layout";
    }
    return "";
}

static std::string GetDefaultProjectDir() {
    std::error_code ec;
    auto cwd = std::filesystem::current_path(ec);
    if (ec) return "projects";
    return (cwd / "projects").string();
}

EditorApp::EditorApp() = default;
EditorApp::~EditorApp() = default;

void EditorApp::OnInit() {
    Logger::Info("UI Editor initialized");

    RenderCommand::SetClearColor(0.08f, 0.08f, 0.1f, 1.0f);
    RenderCommand::SetBlend(true);
    RenderCommand::SetBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    m_renderer = std::make_unique<Renderer>();
    m_renderer->Init();

    m_fontRenderer = std::make_unique<TextRenderer>();
    if (!m_fontRenderer->LoadFont("assets/fonts/arial.ttf", 14.0f)) {
        Logger::Warn("Failed to load font, text will not be rendered");
    }

    InputSystem::GetInstance().SetInputMode(EInputMode::GameAndUI);
    UISystem::GetInstance().SetFontRenderer(m_fontRenderer.get());

    EditorUISystem& ui = m_uiSystem;

    m_canvasView = std::make_unique<CanvasView>();
    m_canvasView->SetFontRenderer(m_fontRenderer.get());
    m_canvasView->OnWidgetClicked([this](Widget* widget) {
        Logger::Info("[Editor] Clicked widget: {}", widget ? typeid(*widget).name() : "null");
        SelectWidget(widget);
    });

    m_menuBar = std::make_unique<MenuBar>();
    m_menuBar->SetFontRenderer(m_fontRenderer.get());
    m_menuBar->OnAction([this](MenuBarAction action) {
        switch (action) {
            case MenuBarAction::FILE_NEW_PROJECT:  ShowNewProjectDialog(); break;
            case MenuBarAction::FILE_OPEN_PROJECT:
            {
                HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
                std::string path = FileDialog::OpenFile("Open Project", "CUI Project\0*.cuiproj\0All\0*.*\0", hwnd);
                if (!path.empty()) OpenProject(path);
                break;
            }
            case MenuBarAction::FILE_NEW_CUI_FILE: ShowNewCuiFileDialog(); break;
            case MenuBarAction::FILE_OPEN_CUI_FILE:
            {
                HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
                std::string path = FileDialog::OpenFile("Open CUI File", "CUI File\0*.cui\0All\0*.*\0", hwnd);
                if (!path.empty()) LoadCuiFileIntoNewTab(path, true);
                break;
            }
            case MenuBarAction::FILE_IMPORT_CUI_FILE: ImportCuiFile(); break;
            case MenuBarAction::FILE_SAVE:   SaveActiveTab(); break;
            case MenuBarAction::FILE_EXIT:   GetWindow()->Close(); break;
            case MenuBarAction::EDIT_UNDO:   UndoRedoStack::GetInstance().Undo(); break;
            case MenuBarAction::EDIT_REDO:   UndoRedoStack::GetInstance().Redo(); break;
            case MenuBarAction::EDIT_DELETE: DeleteSelected(); break;
            default: break;
        }
    });
    ui.Register(m_menuBar.get(), 0);

    m_widgetPalette = std::make_unique<WidgetPalette>();
    m_widgetPalette->SetFontRenderer(m_fontRenderer.get());
    m_widgetPalette->OnAction([this](WidgetPaletteAction action, float mx, float my) {
        auto hit = m_canvasView->GetHitRect();
        if (hit.Contains(mx, my)) {
            Vec3 pos = m_canvasView->ScreenToWorld(Vec2(mx, my));
            Widget* target = UISystem::GetInstance().HitTestScene(pos);
            Node* parent = nullptr;
            if (target && (dynamic_cast<CanvasPanel*>(target) || dynamic_cast<Layout*>(target))) {
                parent = target;
                const Mat4& parentWorld = parent->GetWorldTransform();
                pos = Mat4::Inverse(parentWorld).TransformPoint(pos);
            }
            AddWidgetToScene(GetWidgetType(action), pos, parent);
        }
    });
    ui.Register(m_widgetPalette.get(), 0);

    m_tabBar = std::make_unique<TabBar>();
    m_tabBar->SetFontRenderer(m_fontRenderer.get());
    m_tabBar->OnTabSwitch([this](int index) { SwitchTab(index); });
    m_tabBar->OnTabClose([this](int index) { CloseTab(index); });
    ui.Register(m_tabBar.get(), 2);

    ui.Register(m_canvasView.get(), 3);

    m_propertyPanel = std::make_unique<PropertyPanel>();
    m_propertyPanel->SetFontRenderer(m_fontRenderer.get());
    m_propertyPanel->SetParentHwnd(glfwGetWin32Window(GetWindow()->GetNativeWindow()));
    m_propertyPanel->OnPropertyChanged([this]() { SetTabDirty(); });
    m_propertyPanel->OnNameChanged([this]() {
        SetTabDirty();
        if (m_selectedWidget) {
            m_widgetTreePanel->SelectNode(static_cast<Node*>(m_selectedWidget));
        }
    });
    ui.Register(m_propertyPanel.get(), 2);

    m_widgetTreePanel = std::make_unique<WidgetTreePanel>();
    m_widgetTreePanel->SetFontRenderer(m_fontRenderer.get());
    m_widgetTreePanel->OnSelectionChanged([this](Node* node) {
        SelectWidget(static_cast<Widget*>(node));
    });
    ui.Register(m_widgetTreePanel.get(), 2);

    m_resourcePanel = std::make_unique<ResourcePanel>();
    m_resourcePanel->SetFontRenderer(m_fontRenderer.get());
    m_resourcePanel->OnFileClick([this](const std::string& filename) {
        HandleCuiFileClick(filename);
    });
    m_resourcePanel->OnFileDelete([this](const std::string& filename) {
        DeleteCuiFile(filename);
    });
    m_resourcePanel->OnFileRename([this](const std::string& oldName, const std::string&) {
        ShowRenameDialog(oldName);
    });
    m_resourcePanel->OnProjectAction([this](int action) {
        if (action == 0) {
            ShowNewCuiFileDialog();
        } else if (action == 1) {
            ImportCuiFile();
        }
    });
    ui.Register(m_resourcePanel.get(), 0);

    // Left panel dividers
    m_leftDivider = std::make_unique<PanelDivider>(PanelDivider::Edge::Left);
    m_leftDivider->OnResize([this](float mouseX) {
        float winW = static_cast<float>(GetWindow()->GetWidth());
        m_layoutConfig.leftPanelWidth = std::clamp(mouseX, 80.0f, winW - m_layoutConfig.rightPanelWidth - 80.0f);
        RecalculateLayout(GetWindow()->GetWidth(), GetWindow()->GetHeight());
    });
    m_leftDivider->OnDragEnd([this]() { SaveLayout(); });
    ui.Register(m_leftDivider.get(), 1);

    m_rightDivider = std::make_unique<PanelDivider>(PanelDivider::Edge::Right);
    m_rightDivider->OnResize([this](float mouseX) {
        float winW = static_cast<float>(GetWindow()->GetWidth());
        m_layoutConfig.rightPanelWidth = std::clamp(winW - mouseX, 80.0f, winW - m_layoutConfig.leftPanelWidth - 80.0f);
        RecalculateLayout(GetWindow()->GetWidth(), GetWindow()->GetHeight());
    });
    m_rightDivider->OnDragEnd([this]() { SaveLayout(); });
    ui.Register(m_rightDivider.get(), 1);

    m_leftDivider1 = std::make_unique<PanelDivider>(PanelDivider::Edge::Horizontal);
    m_leftDivider1->OnResize([this](float mouseY) {
        float menuH = m_layoutConfig.menuBarHeight;
        float leftAreaTop = menuH;
        float leftAreaBottom = static_cast<float>(GetWindow()->GetHeight());
        float clampedY = std::clamp(mouseY, leftAreaTop + 40.0f, leftAreaBottom - 40.0f);
        float dividerPos = clampedY - leftAreaTop;
        m_layoutConfig.leftPanelDivider1Y = dividerPos;
        RecalculateLayout(GetWindow()->GetWidth(), GetWindow()->GetHeight());
    });
    m_leftDivider1->OnDragEnd([this]() { SaveLayout(); });
    ui.Register(m_leftDivider1.get(), 1);

    m_leftDivider2 = std::make_unique<PanelDivider>(PanelDivider::Edge::Horizontal);
    m_leftDivider2->OnResize([this](float mouseY) {
        float menuH = m_layoutConfig.menuBarHeight;
        float leftAreaBottom = static_cast<float>(GetWindow()->GetHeight());
        float clampedY = std::clamp(mouseY, menuH + 80.0f, leftAreaBottom - 40.0f);
        float dividerPos = clampedY - menuH;
        m_layoutConfig.leftPanelDivider2Y = dividerPos;
        RecalculateLayout(GetWindow()->GetWidth(), GetWindow()->GetHeight());
    });
    m_leftDivider2->OnDragEnd([this]() { SaveLayout(); });
    ui.Register(m_leftDivider2.get(), 1);

    // Dialogs
    m_newProjectDialog = std::make_unique<NewProjectDialog>();
    m_newProjectDialog->SetFontRenderer(m_fontRenderer.get());
    m_newProjectDialog->OnOK([this]() {
        std::string name = m_newProjectDialog->GetProjectName();
        std::string path = m_newProjectDialog->GetProjectPath();
        if (!name.empty() && !path.empty()) {
            CreateNewProject(name, path);
        }
        m_newProjectDialog->Hide();
    });
    m_newProjectDialog->OnBrowse([this]() {
        HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
        std::string folder = FileDialog::OpenFolder("Select Project Directory", hwnd);
        if (!folder.empty()) {
            m_newProjectDialog->SetProjectPath(folder);
        }
    });
    ui.Register(m_newProjectDialog.get(), -1);

    m_newCuiFileDialog = std::make_unique<NewCuiFileDialog>();
    m_newCuiFileDialog->SetFontRenderer(m_fontRenderer.get());
    m_newCuiFileDialog->OnOK([this]() {
        std::string name = m_newCuiFileDialog->GetFileName();
        if (!name.empty()) {
            if (name.find(".cui") == std::string::npos) {
                name += ".cui";
            }
            CreateNewCuiFile(name);
        }
        m_newCuiFileDialog->Hide();
    });
    ui.Register(m_newCuiFileDialog.get(), -1);

    m_renameDialog = std::make_unique<RenameDialog>();
    m_renameDialog->SetFontRenderer(m_fontRenderer.get());
    m_renameDialog->OnOK([this]() {
        std::string newName = m_renameDialog->GetNewName();
        if (!newName.empty() && !m_pendingRenameFile.empty()) {
            if (newName.find(".cui") == std::string::npos) {
                newName += ".cui";
            }
            RenameCuiFile(m_pendingRenameFile, newName);
            m_pendingRenameFile.clear();
        }
        m_renameDialog->Hide();
    });
    m_renameDialog->OnCancel([this]() {
        m_pendingRenameFile.clear();
        m_renameDialog->Hide();
    });
    ui.Register(m_renameDialog.get(), -1);

    GetWindow()->SetTitle("CLEngine2D UI Editor");

    int winWidth = GetWindow()->GetWidth();
    int winHeight = GetWindow()->GetHeight();
    LoadLayout();
    RecalculateLayout(winWidth, winHeight);

    ShowWelcomeOrRestore();
}

void EditorApp::ShowWelcomeOrRestore() {
    m_editorState = EditorState::LoadFromFile("assets/editor/editor_state.json");

    if (!m_editorState.lastProjectPath.empty() &&
        std::filesystem::exists(m_editorState.lastProjectPath)) {
        OpenProject(m_editorState.lastProjectPath);

        for (const auto& openFile : m_editorState.openFiles) {
            std::string fullPath = m_project.GetCuiFilePath(openFile);
            if (std::filesystem::exists(fullPath)) {
                LoadCuiFileIntoNewTab(fullPath);
            }
        }

        if (!m_editorState.activeFile.empty()) {
            for (size_t i = 0; i < m_tabs.size(); ++i) {
                if (m_tabs[i].name == m_editorState.activeFile) {
                    SwitchTab(static_cast<int>(i));
                    break;
                }
            }
        }
    }
}

void EditorApp::OnUpdate(float deltaTime) {
    CursorManager::Reset();

    // Skip normal input processing if a modal dialog is visible
    if (m_newProjectDialog->IsVisible() || m_newCuiFileDialog->IsVisible() || m_renameDialog->IsVisible()) {
        m_uiSystem.ProcessInput();
        m_uiSystem.UpdatePanels(deltaTime);
    } else {
        m_uiSystem.ProcessInput();
        m_uiSystem.UpdatePanels(deltaTime);

        bool ctrl = RawInput::IsKeyDown(KeyCode::LeftControl) ||
                    RawInput::IsKeyDown(KeyCode::RightControl);
        if (ctrl && RawInput::IsKeyPressed(KeyCode::S)) {
            SaveActiveTab();
        }
    }

    float fps = 1.0f / deltaTime;
    GetWindow()->SetTitle(std::format("CLEngine2D UI Editor - FPS: {:.0f}", fps));
}

void EditorApp::OnRender() {
    RenderCommand::Clear();

    RenderPanel(m_resourcePanel.get());
    RenderPanel(m_widgetPalette.get());
    RenderPanel(m_menuBar.get());
    RenderPanel(m_tabBar.get());
    RenderPanel(m_canvasView.get(), true);
    RenderPanel(m_propertyPanel.get());
    RenderPanel(m_widgetTreePanel.get());
    RenderPanel(m_leftDivider.get());
    RenderPanel(m_rightDivider.get());
    RenderPanel(m_leftDivider1.get());
    RenderPanel(m_leftDivider2.get());

    // Draw dialogs on top so they overlay everything
    if (m_newProjectDialog->IsVisible()) {
        RenderPanel(m_newProjectDialog.get());
    }
    if (m_newCuiFileDialog->IsVisible()) {
        RenderPanel(m_newCuiFileDialog.get());
    }
    if (m_renameDialog->IsVisible()) {
        RenderPanel(m_renameDialog.get());
    }

    RenderCommand::SetViewport(0, 0, GetWindow()->GetWidth(), GetWindow()->GetHeight());
    DrawPanelBorders();
}

void EditorApp::RenderPanel(IEditorPanel* panel, bool useCustomProj) {
    auto r = panel->GetHitRect();
    int winH = GetWindow()->GetHeight();
    int vpY = winH - static_cast<int>(r.y) - static_cast<int>(r.h);
    RenderCommand::SetViewport(static_cast<int>(r.x), vpY,
                               static_cast<int>(r.w), static_cast<int>(r.h));
    if (useCustomProj) {
        m_renderer->BeginScene(m_canvasView->GetProjection());
    } else {
        m_renderer->BeginScene(Mat4::Ortho(0, r.w, r.h, 0, -1, 1));
    }
    panel->OnRender(*m_renderer);
    m_renderer->EndScene();
}

void EditorApp::DrawPanelBorders() {
    float winW = static_cast<float>(GetWindow()->GetWidth());
    float winH = static_cast<float>(GetWindow()->GetHeight());
    const EditorLayoutConfig& cfg = m_layoutConfig;

    Color borderColor(0.45f, 0.45f, 0.5f, 1.0f);
    float t = 1.0f;

    m_renderer->BeginScene(Mat4::Ortho(0, winW, winH, 0, -1, 1));

    // MenuBar bottom edge
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(winW * 0.5f, cfg.menuBarHeight - t * 0.5f, 0.0f)),
        Vec2(winW, t), borderColor);

    // Left panel right edge
    float lx = cfg.leftPanelWidth;
    float sideH = winH - cfg.menuBarHeight;
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(lx - t * 0.5f, cfg.menuBarHeight + sideH * 0.5f, 0.0f)),
        Vec2(t, sideH), borderColor);

    // Right panel left edge
    float rx = winW - cfg.rightPanelWidth;
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(rx - t * 0.5f, cfg.menuBarHeight + sideH * 0.5f, 0.0f)),
        Vec2(t, sideH), borderColor);

    // ResourcePanel bottom edge (divider 1)
    float div1Y = cfg.menuBarHeight + cfg.leftPanelDivider1Y;
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(cfg.leftPanelWidth * 0.5f, div1Y - t * 0.5f, 0.0f)),
        Vec2(cfg.leftPanelWidth, t), borderColor);

    // WidgetPalette bottom edge (divider 2)
    float div2Y = cfg.menuBarHeight + cfg.leftPanelDivider2Y;
    float paletteHeight = m_widgetPalette->CalcDesiredHeight(cfg.leftPanelWidth);
    float palBot = cfg.menuBarHeight + cfg.leftPanelDivider1Y + paletteHeight;
    float actualDiv2Y = (std::max)(palBot, div2Y);
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(cfg.leftPanelWidth * 0.5f, actualDiv2Y - t * 0.5f, 0.0f)),
        Vec2(cfg.leftPanelWidth, t), borderColor);

    m_renderer->EndScene();
}

void EditorApp::OnShutdown() {
    Logger::Info("UI Editor shutting down");

    if (HasUnsavedTabs()) {
        HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
        int result = MessageBoxA(hwnd,
            "There are unsaved projects.\nDo you want to save all?",
            "Unsaved Changes",
            MB_YESNOCANCEL | MB_ICONWARNING);

        if (result == IDYES) {
            for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
                if (m_tabs[i].dirty) {
                    m_activeTabIndex = i;
                    SaveActiveTab();
                }
            }
        } else if (result == IDCANCEL) {
        }
    }

    SaveLayout();
    SaveEditorState();
    m_uiSystem.Clear();
    m_canvasView.reset();
    m_tabBar.reset();
    m_menuBar.reset();
    m_widgetPalette.reset();
    m_propertyPanel.reset();
    m_widgetTreePanel.reset();
    m_resourcePanel.reset();
    m_tabs.clear();
    m_renderer.reset();
}

void EditorApp::SelectWidget(Widget* widget) {
    m_selectedWidget = widget;
    m_propertyPanel->SetTarget(static_cast<Node*>(widget));
    m_canvasView->GetGizmo()->SetTarget(static_cast<Node*>(widget));
    m_widgetTreePanel->SelectNode(static_cast<Node*>(widget));
    if (widget) {
        m_widgetTreePanel->ExpandPathToNode(static_cast<Node*>(widget));
    }
}

void EditorApp::RecalculateLayout(int windowWidth, int windowHeight) {
    const EditorLayoutConfig& config = m_layoutConfig;

    m_menuBar->SetRect(0.0f, 0.0f,
                       static_cast<float>(windowWidth),
                       config.menuBarHeight);

    float leftPanelW = config.leftPanelWidth;
    float leftAreaTop = config.menuBarHeight;
    float leftAreaH = static_cast<float>(windowHeight) - leftAreaTop;

    float div1Y = leftAreaTop + config.leftPanelDivider1Y;
    float resPanelH = config.leftPanelDivider1Y;

    float paletteHeight = m_widgetPalette->CalcDesiredHeight(leftPanelW);
    float div2Y = leftAreaTop + config.leftPanelDivider2Y;
    float paletteActualY = leftAreaTop + resPanelH;
    float paletteAvailableH = config.leftPanelDivider2Y - config.leftPanelDivider1Y;
    float paletteFinalH = (std::min)(paletteHeight, paletteAvailableH - 4.0f);
    if (paletteFinalH < 28.0f) paletteFinalH = 28.0f;

    float treeY = paletteActualY + paletteFinalH;
    float treeH = leftAreaH - resPanelH - paletteFinalH;

    m_resourcePanel->SetRect(
        0.0f,
        leftAreaTop,
        leftPanelW,
        resPanelH);

    m_widgetPalette->SetRect(
        0.0f,
        paletteActualY,
        leftPanelW,
        paletteFinalH);

    m_widgetTreePanel->SetRect(
        0.0f,
        treeY,
        leftPanelW,
        treeH);

    float canvasWidth = static_cast<float>(windowWidth) - leftPanelW - config.rightPanelWidth;
    float canvasHeight = static_cast<float>(windowHeight) - config.menuBarHeight;

    m_tabBar->SetRect(
        leftPanelW,
        config.menuBarHeight,
        canvasWidth,
        TabBar::TAB_BAR_HEIGHT);

    m_canvasView->SetRect(
        leftPanelW,
        config.menuBarHeight + TabBar::TAB_BAR_HEIGHT,
        canvasWidth,
        canvasHeight - TabBar::TAB_BAR_HEIGHT);

    m_propertyPanel->SetRect(
        static_cast<float>(windowWidth) - config.rightPanelWidth,
        config.menuBarHeight,
        config.rightPanelWidth,
        canvasHeight);

    // Vertical dividers
    m_leftDivider->SetEdgeX(leftPanelW);
    m_leftDivider->SetDividerTop(config.menuBarHeight);
    m_leftDivider->SetDividerHeight(canvasHeight);

    float rightEdgeX = static_cast<float>(windowWidth) - config.rightPanelWidth;
    m_rightDivider->SetEdgeX(rightEdgeX);
    m_rightDivider->SetDividerTop(config.menuBarHeight);
    m_rightDivider->SetDividerHeight(canvasHeight);

    // Horizontal dividers in left panel
    m_leftDivider1->SetEdgeX(0.0f);
    m_leftDivider1->SetHorizontalWidth(leftPanelW);
    m_leftDivider1->SetHorizontalY(div1Y);
    m_leftDivider1->SetDividerTop(leftAreaTop);

    m_leftDivider2->SetEdgeX(0.0f);
    m_leftDivider2->SetHorizontalWidth(leftPanelW);
    m_leftDivider2->SetHorizontalY(div2Y);
    m_leftDivider2->SetDividerTop(leftAreaTop);

    // Dialogs - cover full window
    m_newProjectDialog->SetRect(0.0f, 0.0f,
                                static_cast<float>(windowWidth),
                                static_cast<float>(windowHeight));
    m_newCuiFileDialog->SetRect(0.0f, 0.0f,
                                static_cast<float>(windowWidth),
                                static_cast<float>(windowHeight));
    m_renameDialog->SetRect(0.0f, 0.0f,
                            static_cast<float>(windowWidth),
                            static_cast<float>(windowHeight));

    m_menuBar->SetWindowHeight(windowHeight);
    m_widgetPalette->SetWindowHeight(windowHeight);
    m_widgetTreePanel->SetWindowHeight(windowHeight);
    m_propertyPanel->SetWindowHeight(windowHeight);
    m_canvasView->SetWindowHeight(windowHeight);
    m_newProjectDialog->SetWindowHeight(windowHeight);
    m_newCuiFileDialog->SetWindowHeight(windowHeight);
    m_renameDialog->SetWindowHeight(windowHeight);
}

// ---- Project Management ----

void EditorApp::CreateNewProject(const std::string& name, const std::string& directory) {
    if (m_projectOpen) {
        CloseProject();
    }

    CloseAllTabs();

    std::filesystem::path projDir(directory);
    std::error_code ec;
    std::filesystem::create_directories(projDir, ec);
    std::filesystem::create_directories(projDir / "assets", ec);

    m_project.name = name;
    m_project.directory = projDir.string();
    m_project.files.clear();
    m_project.version = 1;

    std::string projFilePath = m_project.GetProjectFilePath();
    if (m_project.SaveToFile(projFilePath)) {
        Logger::Info("Project created at {}", projFilePath);
    }

    m_projectOpen = true;
    UpdateResourcePanel();
    SaveEditorState();
}

void EditorApp::OpenProject(const std::string& projectFilePath) {
    if (m_projectOpen) {
        CloseProject();
    }

    CloseAllTabs();

    Project loaded = Project::LoadFromFile(projectFilePath);
    if (!loaded.IsValid()) {
        Logger::Error("Failed to load project from {}", projectFilePath);
        return;
    }

    m_project = std::move(loaded);
    m_projectOpen = true;
    UpdateResourcePanel();
    SaveEditorState();
    Logger::Info("Project loaded: {}", m_project.name);
}

void EditorApp::CloseProject() {
    if (!m_projectOpen) return;

    if (HasUnsavedTabs()) {
        HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
        int result = MessageBoxA(hwnd,
            "Save changes before closing project?",
            "Unsaved Changes",
            MB_YESNOCANCEL | MB_ICONWARNING);

        if (result == IDYES) {
            for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
                if (m_tabs[i].dirty) {
                    m_activeTabIndex = i;
                    SaveActiveTab();
                }
            }
        } else if (result == IDCANCEL) {
            return;
        }
    }

    CloseAllTabs();
    m_project = Project();
    m_projectOpen = false;
    m_resourcePanel->ClearProject();
    SelectWidget(nullptr);
    m_propertyPanel->SetTarget(nullptr);
    m_canvasView->GetGizmo()->SetTarget(nullptr);
    m_widgetTreePanel->SelectNode(nullptr);
    m_canvasView->SetEditedScene(nullptr);
    UISystem::GetInstance().SetUIRoot(nullptr);
    SaveEditorState();
}

void EditorApp::CreateNewCuiFile(const std::string& filename) {
    if (!m_projectOpen) {
        Logger::Warn("No project open");
        return;
    }

    std::string fullPath = m_project.GetCuiFilePath(filename);

    if (std::filesystem::exists(fullPath)) {
        Logger::Warn("File already exists: {}", fullPath);
        return;
    }

    auto scene = std::make_unique<Scene>();
    auto root = std::make_unique<CanvasPanel>();
    root->SetContentSize(Vec2(800.0f, 600.0f));
    root->SetName("Root");
    root->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
    scene->SetRoot(std::move(root));

    Node* rootPtr = scene->GetRoot();
    if (!UISerializer::SaveToFile(rootPtr, fullPath)) {
        Logger::Error("Failed to save new CUI file: {}", fullPath);
        return;
    }

    m_project.AddFile(filename);
    SaveProjectFile();

    m_activeTabIndex = -1;
    LoadCuiFileIntoNewTab(fullPath);

    UpdateResourcePanel();
    SaveEditorState();
    Logger::Info("Created new CUI file: {}", fullPath);
}

void EditorApp::DeleteCuiFile(const std::string& filename) {
    if (!m_projectOpen) return;

    HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
    std::string msg = "Delete \"" + filename + "\"?";
    int result = MessageBoxA(hwnd, msg.c_str(), "Delete File",
                             MB_YESNO | MB_ICONWARNING);
    if (result != IDYES) return;

    std::string fullPath = m_project.GetCuiFilePath(filename);

    for (int i = static_cast<int>(m_tabs.size()) - 1; i >= 0; --i) {
        if (m_tabs[static_cast<size_t>(i)].filePath == fullPath) {
            CloseTab(i);
            break;
        }
    }

    std::error_code ec;
    std::filesystem::remove(fullPath, ec);

    m_project.RemoveFile(filename);
    SaveProjectFile();
    UpdateResourcePanel();
    SaveEditorState();
    Logger::Info("Deleted CUI file: {}", filename);
}

void EditorApp::RenameCuiFile(const std::string& oldName, const std::string& newName) {
    if (!m_projectOpen) return;

    std::string oldPath = m_project.GetCuiFilePath(oldName);
    std::string newPath = m_project.GetCuiFilePath(newName);

    if (std::filesystem::exists(newPath)) {
        Logger::Warn("Rename target already exists: {}", newPath);
        return;
    }

    std::error_code ec;
    std::filesystem::rename(oldPath, newPath, ec);
    if (ec) {
        Logger::Error("Failed to rename {} to {}: {}", oldPath, newPath, ec.message());
        return;
    }

    for (auto& tab : m_tabs) {
        if (tab.filePath == oldPath) {
            tab.filePath = newPath;
            tab.name = newName;
            break;
        }
    }

    m_project.RenameFile(oldName, newName);
    SaveProjectFile();
    UpdateResourcePanel();
    UpdateTabBar();
    SaveEditorState();
    Logger::Info("Renamed {} to {}", oldName, newName);
}

void EditorApp::ImportCuiFile() {
    if (!m_projectOpen) return;

    HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
    std::string srcPath = FileDialog::OpenFile("Import CUI File", "CUI File\0*.cui\0All\0*.*\0", hwnd);
    if (srcPath.empty()) return;

    std::filesystem::path src(srcPath);
    std::string filename = src.filename().string();

    std::string dstPath = m_project.GetCuiFilePath(filename);
    if (std::filesystem::exists(dstPath)) {
        Logger::Warn("File already exists in project: {}", filename);
        return;
    }

    std::error_code ec;
    std::filesystem::copy(srcPath, dstPath, ec);
    if (ec) {
        Logger::Error("Failed to import file: {}", ec.message());
        return;
    }

    m_project.AddFile(filename);
    SaveProjectFile();
    UpdateResourcePanel();
    SaveEditorState();
    Logger::Info("Imported: {}", filename);
}

void EditorApp::UpdateResourcePanel() {
    if (m_projectOpen) {
        m_resourcePanel->SetProject(&m_project);
    } else {
        m_resourcePanel->ClearProject();
    }
}

void EditorApp::HandleCuiFileClick(const std::string& filename) {
    if (!m_projectOpen) return;

    std::string fullPath = m_project.GetCuiFilePath(filename);

    if (!std::filesystem::exists(fullPath)) {
        Logger::Warn("File not found: {}", fullPath);
        return;
    }

    for (int i = 0; i < static_cast<int>(m_tabs.size()); ++i) {
        if (m_tabs[i].filePath == fullPath) {
            SwitchTab(i);
            return;
        }
    }

    LoadCuiFileIntoNewTab(fullPath);
}

// ---- Dialog Helpers ----

void EditorApp::ShowNewProjectDialog() {
    if (m_newProjectDialog->IsVisible()) return;

    m_newProjectDialog->SetProjectPath(GetDefaultProjectDir());
    // Set default project name via PresetValue
    // (SetProjectPath already uses PresetValue for path)
    m_newProjectDialog->Show();
}

void EditorApp::ShowNewCuiFileDialog() {
    if (m_newCuiFileDialog->IsVisible()) return;
    if (!m_projectOpen) {
        Logger::Warn("No project open. Create or open a project first.");
        return;
    }

    m_newCuiFileDialog->SetFileName("NewFile.cui");
    m_newCuiFileDialog->SetDirectory(m_project.directory);
    m_newCuiFileDialog->Show();
}

void EditorApp::ShowRenameDialog(const std::string& oldName) {
    if (m_renameDialog->IsVisible()) return;
    m_pendingRenameFile = oldName;
    size_t dotPos = oldName.rfind('.');
    std::string nameWithoutExt = (dotPos != std::string::npos) ? oldName.substr(0, dotPos) : oldName;
    m_renameDialog->Show(nameWithoutExt);
}

// ---- State Persistence ----

void EditorApp::SaveProjectFile() {
    if (!m_projectOpen) return;
    m_project.SaveToFile(m_project.GetProjectFilePath());
}

void EditorApp::SaveLayout() {
    std::string filepath = "assets/editor/editor_layout.json";
    if (m_layoutConfig.SaveToFile(filepath)) {
        Logger::Info("Editor layout saved to {}", filepath);
    } else {
        Logger::Error("Failed to save editor layout");
    }
}

void EditorApp::LoadLayout() {
    std::string filepath = "assets/editor/editor_layout.json";
    if (EditorLayoutConfig::LoadFromFile(filepath, m_layoutConfig)) {
        Logger::Info("Editor layout loaded from {}", filepath);
    } else {
        Logger::Info("No saved editor layout found, using defaults");
    }
}

void EditorApp::SaveEditorState() {
    m_editorState.openFiles.clear();
    for (const auto& tab : m_tabs) {
        if (m_projectOpen) {
            std::filesystem::path fp(tab.filePath);
            m_editorState.openFiles.push_back(fp.filename().string());
        } else {
            m_editorState.openFiles.push_back(tab.filePath);
        }
    }

    if (m_activeTabIndex >= 0 && m_activeTabIndex < static_cast<int>(m_tabs.size())) {
        std::filesystem::path fp(m_tabs[m_activeTabIndex].filePath);
        m_editorState.activeFile = fp.filename().string();
    } else {
        m_editorState.activeFile.clear();
    }

    m_editorState.lastProjectPath = m_projectOpen
        ? m_project.GetProjectFilePath()
        : "";

    std::error_code ec;
    std::filesystem::create_directories("assets/editor", ec);
    m_editorState.SaveToFile("assets/editor/editor_state.json");
}

void EditorApp::LoadEditorState() {
    m_editorState = EditorState::LoadFromFile("assets/editor/editor_state.json");
}

// ---- Tab Management ----

void EditorApp::NewTab() {
    auto scene = std::make_unique<Scene>();
    scene->SetRoot(nullptr);

    static int untitledCounter = 0;
    std::string name = (untitledCounter == 0)
        ? "Untitled"
        : "Untitled " + std::to_string(untitledCounter);
    ++untitledCounter;

    EditorTab tab;
    tab.name = name;
    tab.filePath = "assets/ui/" + name + ".cui";
    tab.scene = std::move(scene);
    tab.dirty = false;

    m_tabs.push_back(std::move(tab));
    SwitchTab(static_cast<int>(m_tabs.size()) - 1);
}

void EditorApp::SwitchTab(int index) {
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return;

    m_activeTabIndex = index;
    EditorTab& tab = m_tabs[index];

    Node* root = tab.scene->GetRoot();
    m_widgetTreePanel->SetRoot(root);
    m_canvasView->SetEditedScene(tab.scene.get());
    UISystem::GetInstance().SetUIRoot(static_cast<Widget*>(root));
    SelectWidget(nullptr);

    UpdateTabBar();
    UpdateResourcePanel();

    if (m_projectOpen && !tab.external) {
        std::filesystem::path fp(tab.filePath);
        m_resourcePanel->SetSelectedFile(fp.filename().string());
    }
}

void EditorApp::CloseTab(int index) {
    if (index < 0 || index >= static_cast<int>(m_tabs.size())) return;
    if (m_tabs.size() <= 1) return;

    if (m_tabs[index].dirty) {
        HWND hwnd = glfwGetWin32Window(GetWindow()->GetNativeWindow());
        std::string msg = "Save changes to \"" + m_tabs[index].name + "\"?";
        int result = MessageBoxA(hwnd, msg.c_str(), "Unsaved Changes",
                                 MB_YESNOCANCEL | MB_ICONWARNING);
        if (result == IDYES) {
            m_activeTabIndex = index;
            SaveActiveTab();
        } else if (result == IDCANCEL) {
            return;
        }
    }

    m_tabs.erase(m_tabs.begin() + index);

    int newIndex = index;
    if (newIndex >= static_cast<int>(m_tabs.size())) {
        newIndex = static_cast<int>(m_tabs.size()) - 1;
    }
    SwitchTab(newIndex);

    SaveEditorState();
}

void EditorApp::CloseAllTabs() {
    while (m_tabs.size() > 0) {
        m_tabs.clear();
    }
    m_activeTabIndex = -1;
    m_tabBar->SetTabs({});
    m_tabBar->SetActiveTab(-1);
    SelectWidget(nullptr);
}

void EditorApp::SaveActiveTab() {
    if (m_activeTabIndex < 0 || m_activeTabIndex >= static_cast<int>(m_tabs.size())) return;

    EditorTab& tab = m_tabs[m_activeTabIndex];
    Node* root = tab.scene->GetRoot();
    if (!root) {
        Logger::Warn("Nothing to save — scene is empty");
        return;
    }

    std::filesystem::create_directories(
        std::filesystem::path(tab.filePath).parent_path());
    if (UISerializer::SaveToFile(root, tab.filePath)) {
        Logger::Info("Tab saved to {}", tab.filePath);
        tab.dirty = false;
        UpdateTabBar();
    } else {
        Logger::Error("Failed to save tab {}", tab.filePath);
    }
}

void EditorApp::OpenCuiFile(const std::string& filepath) {
    LoadCuiFileIntoNewTab(filepath);
}

void EditorApp::LoadCuiFileIntoNewTab(const std::string& filepath, bool external) {
    Node* loaded = UISerializer::LoadFromFile(filepath);
    if (!loaded) {
        Logger::Error("Failed to load CUI file: {}", filepath);
        return;
    }

    auto scene = std::make_unique<Scene>();
    scene->SetRoot(std::unique_ptr<Node>(loaded));

    std::string name = filepath.substr(filepath.find_last_of("/\\") + 1);

    EditorTab tab;
    tab.name = name;
    tab.filePath = filepath;
    tab.scene = std::move(scene);
    tab.dirty = false;
    tab.external = external;

    m_tabs.push_back(std::move(tab));
    SwitchTab(static_cast<int>(m_tabs.size()) - 1);
}

void EditorApp::SetTabDirty() {
    if (m_activeTabIndex >= 0 && m_activeTabIndex < static_cast<int>(m_tabs.size())) {
        m_tabs[m_activeTabIndex].dirty = true;
        UpdateTabBar();
    }
}

bool EditorApp::HasUnsavedTabs() const {
    for (const auto& tab : m_tabs) {
        if (tab.dirty) return true;
    }
    return false;
}

bool EditorApp::IsFileOpenInTab(const std::string& filepath) const {
    for (const auto& tab : m_tabs) {
        if (tab.filePath == filepath) return true;
    }
    return false;
}

void EditorApp::SyncTabToPanels() {
    if (m_activeTabIndex < 0 || m_activeTabIndex >= static_cast<int>(m_tabs.size())) return;

    EditorTab& tab = m_tabs[m_activeTabIndex];
    m_canvasView->SetEditedScene(tab.scene.get());
    m_widgetTreePanel->SetRoot(tab.scene->GetRoot());
}

void EditorApp::UpdateTabBar() {
    std::vector<TabBar::TabInfo> infos;
    for (const auto& tab : m_tabs) {
        infos.push_back({ tab.name, tab.dirty, tab.external, tab.filePath });
    }
    m_tabBar->SetTabs(infos);
    m_tabBar->SetActiveTab(m_activeTabIndex);
}

// ---- Widget Operations ----

void EditorApp::AddWidgetToScene(const std::string& type, const Vec3& position, Node* parentOverride) {
    if (m_activeTabIndex < 0 || m_activeTabIndex >= static_cast<int>(m_tabs.size())) return;

    Scene* scene = m_tabs[m_activeTabIndex].scene.get();
    if (!scene) return;

    std::unique_ptr<Node> widget;

    if (type == "Button") {
        auto btn = std::make_unique<Button>();
        btn->SetText("New Button");
        widget = std::move(btn);
    } else if (type == "Label") {
        auto lbl = std::make_unique<Label>();
        lbl->SetText("New Label");
        widget = std::move(lbl);
    } else if (type == "Image") {
        widget = std::make_unique<Image>();
    } else if (type == "Panel") {
        widget = std::make_unique<CanvasPanel>();
    } else if (type == "Layout") {
        auto layout = std::make_unique<Layout>();
        layout->SetLayoutType(Layout::Type::VERTICAL);
        widget = std::move(layout);
    }

    if (!widget) return;

    static int counter = 0;
    widget->SetName(type + std::to_string(counter++));

    if (!scene->GetRoot()) {
        widget->SetContentSize(Vec2(800.0f, 600.0f));
        widget->SetPosition(Vec3(0.0f, 0.0f, 0.0f));
        scene->SetRoot(std::move(widget));

        Node* root = scene->GetRoot();
        m_widgetTreePanel->SetRoot(root);
        m_canvasView->SetEditedScene(scene);
        UISystem::GetInstance().SetUIRoot(static_cast<Widget*>(root));
    } else {
        Node* parent = parentOverride ? parentOverride : scene->GetRoot();
        widget->SetZOrder(static_cast<int>(parent->GetChildCount()));
        widget->SetContentSize(Vec2(150.0f, 40.0f));
        widget->SetPosition(position);
        parent->AddChild(std::move(widget));

        if (auto* layout = dynamic_cast<Layout*>(parent)) {
            layout->DoLayout();
        }
    }

    SetTabDirty();
    Logger::Info("Added widget: {}", type);
}

void EditorApp::DeleteSelected() {
    if (!m_selectedWidget) return;

    Node* parent = m_selectedWidget->GetParent();
    if (parent) {
        parent->RemoveChild(m_selectedWidget);
        SelectWidget(nullptr);
        SetTabDirty();
    }
}

void EditorApp::OnWindowResize(int width, int height) {
    RecalculateLayout(width, height);
}
