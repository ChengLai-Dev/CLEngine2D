#include "EditorApp.h"
#include "EditorUISystem.h"
#include "CanvasView.h"
#include "MenuBar.h"
#include "WidgetPalette.h"
#include "PropertyPanel.h"
#include "WidgetTreePanel.h"
#include "Gizmo.h"
#include "Serializer.h"
#include "UndoRedo.h"
#include "TabBar.h"

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
            case MenuBarAction::FILE_NEW:    NewTab(); break;
            case MenuBarAction::FILE_SAVE:   SaveActiveTab(); break;
            case MenuBarAction::FILE_OPEN:   LoadSceneIntoNewTab(); break;
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
            AddWidgetToScene(GetWidgetType(action), pos);
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

    GetWindow()->SetTitle("CLEngine2D UI Editor");

    int winWidth = GetWindow()->GetWidth();
    int winHeight = GetWindow()->GetHeight();
    LoadLayout();
    RecalculateLayout(winWidth, winHeight);

    NewTab();
}

void EditorApp::OnUpdate(float deltaTime) {
    CursorManager::Reset();
    m_uiSystem.ProcessInput();
    m_uiSystem.UpdatePanels(deltaTime);

    bool ctrl = RawInput::IsKeyDown(KeyCode::LeftControl) ||
                RawInput::IsKeyDown(KeyCode::RightControl);
    if (ctrl && RawInput::IsKeyPressed(KeyCode::S)) {
        SaveActiveTab();
    }

    float fps = 1.0f / deltaTime;
    GetWindow()->SetTitle(std::format("CLEngine2D UI Editor - FPS: {:.0f}", fps));
}

void EditorApp::OnRender() {
    RenderCommand::Clear();

    RenderPanel(m_widgetPalette.get());
    RenderPanel(m_menuBar.get());
    RenderPanel(m_tabBar.get());
    RenderPanel(m_canvasView.get(), true);
    RenderPanel(m_propertyPanel.get());
    RenderPanel(m_widgetTreePanel.get());
    RenderPanel(m_leftDivider.get());
    RenderPanel(m_rightDivider.get());

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

    // 1. MenuBar bottom edge (full width)
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(winW * 0.5f, cfg.menuBarHeight - t * 0.5f, 0.0f)),
        Vec2(winW, t), borderColor);

    // 2. Left panel right edge (vertical, from menuBar bottom to window bottom)
    float lx = cfg.leftPanelWidth;
    float sideH = winH - cfg.menuBarHeight;
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(lx - t * 0.5f, cfg.menuBarHeight + sideH * 0.5f, 0.0f)),
        Vec2(t, sideH), borderColor);

    // 3. Property panel left edge (vertical, from menuBar bottom to window bottom)
    float rx = winW - cfg.rightPanelWidth;
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(rx - t * 0.5f, cfg.menuBarHeight + sideH * 0.5f, 0.0f)),
        Vec2(t, sideH), borderColor);

    // 4. WidgetPalette bottom edge (horizontal, within left panel)
    auto palRect = m_widgetPalette->GetHitRect();
    float palBot = palRect.y + palRect.h;
    m_renderer->DrawQuad(
        Mat4::Translate(Vec3(cfg.leftPanelWidth * 0.5f, palBot - t * 0.5f, 0.0f)),
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
            // Cancel shutdown - actually we can't cancel OnShutdown
            // Just don't save, let them lose changes
        }
    }

    SaveLayout();
    m_uiSystem.Clear();
    m_canvasView.reset();
    m_tabBar.reset();
    m_menuBar.reset();
    m_widgetPalette.reset();
    m_propertyPanel.reset();
    m_widgetTreePanel.reset();
    m_tabs.clear();
    m_renderer.reset();
}

void EditorApp::SelectWidget(Widget* widget) {
    m_selectedWidget = widget;
    m_propertyPanel->SetTarget(static_cast<Node*>(widget));
    m_canvasView->GetGizmo()->SetTarget(static_cast<Node*>(widget));
    m_widgetTreePanel->SelectNode(static_cast<Node*>(widget));
}

void EditorApp::RecalculateLayout(int windowWidth, int windowHeight) {
    const EditorLayoutConfig& config = m_layoutConfig;

    m_menuBar->SetRect(0.0f,
                       0.0f,
                       static_cast<float>(windowWidth),
                       config.menuBarHeight);

    float paletteHeight = m_widgetPalette->CalcDesiredHeight(config.leftPanelWidth);
    m_widgetPalette->SetRect(
        0.0f,
        config.menuBarHeight,
        config.leftPanelWidth,
        paletteHeight);

    m_widgetTreePanel->SetRect(
        0.0f,
        config.menuBarHeight + paletteHeight,
        config.leftPanelWidth,
        static_cast<float>(windowHeight) - config.menuBarHeight - paletteHeight);

    float canvasWidth = static_cast<float>(windowWidth) - config.leftPanelWidth - config.rightPanelWidth;
    float canvasHeight = static_cast<float>(windowHeight) - config.menuBarHeight;

    m_tabBar->SetRect(
        config.leftPanelWidth,
        config.menuBarHeight,
        canvasWidth,
        TabBar::TAB_BAR_HEIGHT);

    m_canvasView->SetRect(
        config.leftPanelWidth,
        config.menuBarHeight + TabBar::TAB_BAR_HEIGHT,
        canvasWidth,
        canvasHeight - TabBar::TAB_BAR_HEIGHT);

    m_propertyPanel->SetRect(
        static_cast<float>(windowWidth) - config.rightPanelWidth,
        config.menuBarHeight,
        config.rightPanelWidth,
        canvasHeight);

    m_leftDivider->SetEdgeX(config.leftPanelWidth);
    m_leftDivider->SetDividerTop(config.menuBarHeight);
    m_leftDivider->SetDividerHeight(canvasHeight);

    float rightEdgeX = static_cast<float>(windowWidth) - config.rightPanelWidth;
    m_rightDivider->SetEdgeX(rightEdgeX);
    m_rightDivider->SetDividerTop(config.menuBarHeight);
    m_rightDivider->SetDividerHeight(canvasHeight);

    m_menuBar->SetWindowHeight(windowHeight);
    m_widgetPalette->SetWindowHeight(windowHeight);
    m_widgetTreePanel->SetWindowHeight(windowHeight);
    m_propertyPanel->SetWindowHeight(windowHeight);
    m_canvasView->SetWindowHeight(windowHeight);
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
    tab.filePath = "assets/ui/" + name + ".ui";
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
    if (Serializer::SaveToFile(root, tab.filePath)) {
        Logger::Info("Tab saved to {}", tab.filePath);
        tab.dirty = false;
        UpdateTabBar();
    } else {
        Logger::Error("Failed to save tab {}", tab.filePath);
    }
}

void EditorApp::LoadSceneIntoNewTab() {
    std::string filepath = "assets/ui/editor_test.ui";
    Node* loaded = Serializer::LoadFromFile(filepath);
    if (!loaded) {
        Logger::Error("Failed to load scene");
        return;
    }

    auto scene = std::make_unique<Scene>();
    scene->SetRoot(std::unique_ptr<Node>(loaded));

    EditorTab tab;
    tab.name = filepath.substr(filepath.find_last_of("/\\") + 1);
    tab.filePath = filepath;
    tab.scene = std::move(scene);
    tab.dirty = false;

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

void EditorApp::SyncTabToPanels() {
    if (m_activeTabIndex < 0 || m_activeTabIndex >= static_cast<int>(m_tabs.size())) return;

    EditorTab& tab = m_tabs[m_activeTabIndex];
    m_canvasView->SetEditedScene(tab.scene.get());
    m_widgetTreePanel->SetRoot(tab.scene->GetRoot());
}

void EditorApp::UpdateTabBar() {
    std::vector<TabBar::TabInfo> infos;
    for (const auto& tab : m_tabs) {
        infos.push_back({ tab.name, tab.dirty });
    }
    m_tabBar->SetTabs(infos);
    m_tabBar->SetActiveTab(m_activeTabIndex);
}

// ---- Modified existing functions ----

void EditorApp::AddWidgetToScene(const std::string& type, const Vec3& position) {
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
        Node* root = scene->GetRoot();
        widget->SetZOrder(static_cast<int>(root->GetChildCount()));
        widget->SetContentSize(Vec2(150.0f, 40.0f));
        widget->SetPosition(position);
        root->AddChild(std::move(widget));
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

void EditorApp::OnWindowResize(int width, int height) {
    RecalculateLayout(width, height);
}
