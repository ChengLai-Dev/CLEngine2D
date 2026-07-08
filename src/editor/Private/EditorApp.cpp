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
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/Layout.h>
#include <SceneGraph/UISystem.h>
#include <Input/InputSystem.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Platform/Window.h>
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

    InputSystem::GetInstance().SetInputMode(EInputMode::GameAndUI);
    UISystem::GetInstance().SetUIRoot(static_cast<Widget*>(m_editedScene->GetRoot()));

    EditorUISystem& ui = m_uiSystem;

    m_canvasView = std::make_unique<CanvasView>();
    m_canvasView->SetEditedScene(m_editedScene.get());
    m_canvasView->OnWidgetClicked([this](Widget* widget) {
        Logger::Info("[Editor] Clicked widget: {}", widget ? typeid(*widget).name() : "null");
        SelectWidget(widget);
    });
    ui.Register(m_canvasView.get(), 3);

    m_menuBar = std::make_unique<MenuBar>();
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
    m_widgetPalette->OnAction([this](WidgetPaletteAction action, float mx, float my) {
        auto hit = m_canvasView->GetHitRect();
        if (hit.Contains(mx, my)) {
            Vec3 pos = m_canvasView->ScreenToWorld(Vec2(mx, my));
            AddWidgetToScene(GetWidgetType(action), pos);
        }
    });
    ui.Register(m_widgetPalette.get(), 0);

    m_propertyPanel = std::make_unique<PropertyPanel>();
    ui.Register(m_propertyPanel.get(), 2);

    m_widgetTreePanel = std::make_unique<WidgetTreePanel>();
    m_widgetTreePanel->SetRoot(m_editedScene->GetRoot());
    m_widgetTreePanel->OnSelectionChanged([this](Node* node) {
        SelectWidget(static_cast<Widget*>(node));
    });
    ui.Register(m_widgetTreePanel.get(), 2);

    GetWindow()->SetTitle("CLEngine2D UI Editor");

    int winWidth = GetWindow()->GetWidth();
    int winHeight = GetWindow()->GetHeight();
    m_editorCamera = std::make_unique<OrthographicCamera>(0.0f, static_cast<float>(winWidth), static_cast<float>(winHeight), 0.0f);
    RecalculateLayout(winWidth, winHeight);
}

void EditorApp::OnUpdate(float deltaTime) {
    m_uiSystem.ProcessInput();
    m_uiSystem.UpdatePanels(deltaTime);
}

void EditorApp::OnRender() {
    RenderCommand::Clear();

    m_renderer->BeginScene(*m_editorCamera);

    m_uiSystem.RenderPanels(*m_renderer);

    m_renderer->EndScene();

    RenderCommand::SetViewport(0, 0, GetWindow()->GetWidth(), GetWindow()->GetHeight());
}

void EditorApp::OnShutdown() {
    Logger::Info("UI Editor shutting down");
    m_uiSystem.Clear();
    m_canvasView.reset();
    m_menuBar.reset();
    m_widgetPalette.reset();
    m_propertyPanel.reset();
    m_widgetTreePanel.reset();
    m_editedScene.reset();
    m_renderer.reset();
    m_editorCamera.reset();
}

void EditorApp::SelectWidget(Widget* widget) {
    m_selectedWidget = widget;
    m_propertyPanel->SetTarget(static_cast<Node*>(widget));
    m_canvasView->GetGizmo()->SetTarget(static_cast<Node*>(widget));
}

void EditorApp::RecalculateLayout(int windowWidth, int windowHeight) {
    const float menuBarHeight = 24.0f;
    const float paletteHeight = 50.0f;
    const float leftPanelWidth = 250.0f;
    const float rightPanelWidth = 300.0f;

    float ww = static_cast<float>(windowWidth);
    float wh = static_cast<float>(windowHeight);

    m_menuBar->SetRect(0.0f, 0.0f, ww, menuBarHeight);
    m_widgetPalette->SetRect(0.0f, menuBarHeight, leftPanelWidth, paletteHeight);
    m_widgetTreePanel->SetRect(0.0f, menuBarHeight + paletteHeight, leftPanelWidth,
                                wh - menuBarHeight - paletteHeight);
    m_propertyPanel->SetRect(ww - rightPanelWidth, menuBarHeight, rightPanelWidth,
                              wh - menuBarHeight);
    m_canvasView->SetRect(leftPanelWidth, menuBarHeight,
                          ww - leftPanelWidth - rightPanelWidth,
                          wh - menuBarHeight);

    m_uiSystem.SetAllWindowHeight(windowHeight);

    m_editorCamera->SetProjection(0.0f, ww, wh, 0.0f);
}

void EditorApp::OnWindowResize(int width, int height) {
    RecalculateLayout(width, height);
}

void EditorApp::AddWidgetToScene(const std::string& type, const Vec3& position) {
    if (!m_editedScene) return;

    std::unique_ptr<Widget> widget;

    if (type == "Button") {
        auto btn = std::make_unique<Button>();
        btn->SetText("New Button");
        widget = std::move(btn);
    } else if (type == "Label") {
        auto lbl = std::make_unique<Label>();
        lbl->SetText("New Label");
        widget = std::move(lbl);
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
