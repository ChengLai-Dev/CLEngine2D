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
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Platform/Window.h>
#include <TextRenderer.h>
#include <Logger.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

    m_editedScene = std::make_unique<Scene>();
    auto canvasPanel = std::make_unique<CanvasPanel>();
    canvasPanel->SetContentSize(Vec2(800.0f, 600.0f));
    canvasPanel->SetName("Root");
    m_editedScene->SetRoot(std::move(canvasPanel));

    m_fontRenderer = std::make_unique<TextRenderer>();
    if (!m_fontRenderer->LoadFont("assets/fonts/arial.ttf", 14.0f)) {
        Logger::Warn("Failed to load font, text will not be rendered");
    }

    InputSystem::GetInstance().SetInputMode(EInputMode::GameAndUI);
    UISystem::GetInstance().SetUIRoot(static_cast<Widget*>(m_editedScene->GetRoot()));
    UISystem::GetInstance().SetFontRenderer(m_fontRenderer.get());

    EditorUISystem& ui = m_uiSystem;

    m_canvasView = std::make_unique<CanvasView>();
    m_canvasView->SetEditedScene(m_editedScene.get());
    m_canvasView->SetFontRenderer(m_fontRenderer.get());
    m_canvasView->OnWidgetClicked([this](Widget* widget) {
        Logger::Info("[Editor] Clicked widget: {}", widget ? typeid(*widget).name() : "null");
        SelectWidget(widget);
    });
    ui.Register(m_canvasView.get(), 3);

    m_menuBar = std::make_unique<MenuBar>();
    m_menuBar->SetFontRenderer(m_fontRenderer.get());
    m_menuBar->OnAction([this](MenuBarAction action) {
        switch (action) {
            case MenuBarAction::FILE_SAVE: SaveScene(); break;
            case MenuBarAction::FILE_OPEN: LoadScene(); break;
            case MenuBarAction::EDIT_UNDO: UndoRedoStack::GetInstance().Undo(); break;
            case MenuBarAction::EDIT_REDO: UndoRedoStack::GetInstance().Redo(); break;
            case MenuBarAction::EDIT_DELETE: DeleteSelected(); break;
            default: break;
        }
    });
    ui.Register(m_menuBar.get(), 1);

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

    m_propertyPanel = std::make_unique<PropertyPanel>();
    m_propertyPanel->SetFontRenderer(m_fontRenderer.get());
    m_propertyPanel->OnNameChanged([this]() {
        if (m_selectedWidget) {
            m_widgetTreePanel->SelectNode(static_cast<Node*>(m_selectedWidget));
        }
    });
    ui.Register(m_propertyPanel.get(), 2);

    m_widgetTreePanel = std::make_unique<WidgetTreePanel>();
    m_widgetTreePanel->SetFontRenderer(m_fontRenderer.get());
    m_widgetTreePanel->SetRoot(m_editedScene->GetRoot());
    m_widgetTreePanel->OnSelectionChanged([this](Node* node) {
        SelectWidget(static_cast<Widget*>(node));
    });
    ui.Register(m_widgetTreePanel.get(), 2);

    GetWindow()->SetTitle("CLEngine2D UI Editor");

    int winWidth = GetWindow()->GetWidth();
    int winHeight = GetWindow()->GetHeight();
    LoadLayout();
    RecalculateLayout(winWidth, winHeight);
}

void EditorApp::OnUpdate(float deltaTime) {
    m_uiSystem.ProcessInput();
    m_uiSystem.UpdatePanels(deltaTime);

    float fps = 1.0f / deltaTime;
    GetWindow()->SetTitle(std::format("CLEngine2D UI Editor - FPS: {:.0f}", fps));
}

void EditorApp::OnRender() {
    RenderCommand::Clear();

    RenderPanel(m_widgetPalette.get());
    RenderPanel(m_menuBar.get());
    RenderPanel(m_propertyPanel.get());
    RenderPanel(m_widgetTreePanel.get());
    RenderPanel(m_canvasView.get(), true);

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
    SaveLayout();
    m_uiSystem.Clear();
    m_canvasView.reset();
    m_menuBar.reset();
    m_widgetPalette.reset();
    m_propertyPanel.reset();
    m_widgetTreePanel.reset();
    m_editedScene.reset();
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

    m_propertyPanel->SetRect(
        static_cast<float>(windowWidth) - config.rightPanelWidth,
        config.menuBarHeight,
        config.rightPanelWidth,
        static_cast<float>(windowHeight) - config.menuBarHeight);

    m_canvasView->SetRect(
        config.leftPanelWidth,
        config.menuBarHeight,
        static_cast<float>(windowWidth) - config.leftPanelWidth - config.rightPanelWidth,
        static_cast<float>(windowHeight) - config.menuBarHeight);

    m_menuBar->SetWindowHeight(windowHeight);
    m_widgetPalette->SetWindowHeight(windowHeight);
    m_widgetTreePanel->SetWindowHeight(windowHeight);
    m_propertyPanel->SetWindowHeight(windowHeight);
    m_canvasView->SetWindowHeight(windowHeight);
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

void EditorApp::AddWidgetToScene(const std::string& type, const Vec3& position) {
    if (!m_editedScene) return;

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
    widget->SetContentSize(Vec2(150.0f, 40.0f));
    widget->SetPosition(position);

    m_editedScene->GetRoot()->AddChild(std::move(widget));

    Logger::Info("Added widget: {}", type);
}

void EditorApp::DeleteSelected() {
    if (!m_selectedWidget) return;

    Node* parent = m_selectedWidget->GetParent();
    if (parent) {
        parent->RemoveChild(m_selectedWidget);
        SelectWidget(nullptr);
    }
}

void EditorApp::SaveScene() {
    Node* root = m_editedScene->GetRoot();
    std::string filepath = "assets/ui/editor_test.ui";
    if (Serializer::SaveToFile(root, filepath)) {
        Logger::Info("Scene saved to {}", filepath);
    } else {
        Logger::Error("Failed to save scene");
    }
}

void EditorApp::LoadScene() {
    std::string filepath = "assets/ui/editor_test.ui";
    Node* loaded = Serializer::LoadFromFile(filepath);
    if (loaded) {
        m_editedScene->SetRoot(std::unique_ptr<Node>(loaded));
        m_widgetTreePanel->SetRoot(m_editedScene->GetRoot());
        m_canvasView->SetEditedScene(m_editedScene.get());
        SelectWidget(nullptr);
        Logger::Info("Scene loaded");
    } else {
        Logger::Error("Failed to load scene");
    }
}
