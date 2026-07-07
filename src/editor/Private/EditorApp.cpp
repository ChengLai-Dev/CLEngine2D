#include "EditorApp.h"
#include "CanvasView.h"
#include "PropertyPanel.h"
#include "WidgetTreePanel.h"
#include "Toolbar.h"
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
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Platform/Window.h>
#include <Input/RawInput.h>
#include <Input/InputCodes.h>
#include <Logger.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

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

    m_canvasView = std::make_unique<CanvasView>();
    m_canvasView->SetEditedScene(m_editedScene.get());

    m_propertyPanel = std::make_unique<PropertyPanel>();

    m_widgetTreePanel = std::make_unique<WidgetTreePanel>();
    m_widgetTreePanel->SetRoot(m_editedScene->GetRoot());

    m_widgetTreePanel->OnSelectionChanged([this](Node* node) {
        SelectNode(node);
    });

    m_toolbar = std::make_unique<Toolbar>();

    m_toolbar->OnAction([this](ToolbarAction action) {
        OnToolbarAction(static_cast<int>(action));
    });

    GetWindow()->SetTitle("CLEngine2D UI Editor");

    int winW = GetWindow()->GetWidth();
    int winH = GetWindow()->GetHeight();
    m_editorCamera = std::make_unique<OrthographicCamera>(0.0f, static_cast<float>(winW), static_cast<float>(winH), 0.0f);
    RecalculateLayout(winW, winH);
}

void EditorApp::OnUpdate(float deltaTime) {
    auto [mx, my] = RawInput::GetMousePosition();
    Vec2 mousePos(mx, my);

    bool leftPressed = RawInput::IsMouseButtonPressed(MouseCode::ButtonLeft);
    bool leftDown = RawInput::IsMouseButtonDown(MouseCode::ButtonLeft);
    bool leftReleased = RawInput::IsMouseButtonReleased(MouseCode::ButtonLeft);
    bool rightDown = RawInput::IsMouseButtonDown(MouseCode::ButtonRight);

    float scrollY = RawInput::GetScrollDeltaY();

    if (scrollY != 0.0f) {
        m_canvasView->Zoom(scrollY > 0.0f ? 1.1f : 0.9f);
    }

    float canvasX = m_canvasView->GetViewX();
    float canvasY = m_canvasView->GetViewY();
    float canvasW = m_canvasView->GetViewW();
    float canvasH = m_canvasView->GetViewH();
    bool inCanvas = mx >= canvasX && mx < canvasX + canvasW &&
                    my >= canvasY && my < canvasY + canvasH;

    Gizmo* gizmo = m_canvasView->GetGizmo();

    if (inCanvas) {
        Vec3 worldPos = m_canvasView->ScreenToWorld(mousePos);

        if (leftPressed) {
            if (!gizmo->IsDragging()) {
                GizmoHandle::Type handle = gizmo->HitTestHandle(worldPos);
                if (handle != GizmoHandle::NONE) {
                    gizmo->BeginDrag(handle, worldPos);
                    m_isDragging = true;
                } else {
                    SelectNode(nullptr);
                }
            }
        } else if (leftDown && m_isDragging && gizmo->IsDragging()) {
            Vec3 currentWorld = m_canvasView->ScreenToWorld(mousePos);
            gizmo->Drag(currentWorld);
        } else if (leftReleased) {
            if (gizmo->IsDragging()) {
                gizmo->EndDrag();
            }
            m_isDragging = false;
        }
    }

    if (rightDown && inCanvas) {
        Vec2 delta = mousePos - m_lastMousePos;
        m_canvasView->Pan(Vec2(-delta.x, delta.y) * 0.5f);
    }

    m_lastMousePos = mousePos;

    m_canvasView->OnUpdate(deltaTime);
}

void EditorApp::OnRender() {
    RenderCommand::Clear();

    m_renderer->BeginScene(*m_editorCamera);

    m_canvasView->OnRender(*m_renderer);
    m_propertyPanel->OnRender(*m_renderer);
    m_widgetTreePanel->OnRender(*m_renderer);
    m_toolbar->OnRender(*m_renderer);

    m_renderer->EndScene();

    RenderCommand::SetViewport(0, 0, GetWindow()->GetWidth(), GetWindow()->GetHeight());
}

void EditorApp::OnShutdown() {
    Logger::Info("UI Editor shutting down");
    m_canvasView.reset();
    m_propertyPanel.reset();
    m_widgetTreePanel.reset();
    m_toolbar.reset();
    m_editedScene.reset();
    m_renderer.reset();
    m_editorCamera.reset();
}

void EditorApp::SelectNode(Node* node) {
    m_selectedNode = node;
    m_propertyPanel->SetTarget(node);
    m_canvasView->GetGizmo()->SetTarget(node);
}

void EditorApp::RecalculateLayout(int windowW, int windowH) {
    const float toolbarH = 36.0f;
    const float leftPanelW = 250.0f;
    const float rightPanelW = 300.0f;

    m_toolbar->SetRect(0.0f, 0.0f, static_cast<float>(windowW), toolbarH);

    m_widgetTreePanel->SetRect(0.0f, toolbarH, leftPanelW,
                               static_cast<float>(windowH) - toolbarH);

    m_propertyPanel->SetRect(static_cast<float>(windowW) - rightPanelW, toolbarH,
                             rightPanelW, static_cast<float>(windowH) - toolbarH);

    m_canvasView->SetViewRect(leftPanelW, toolbarH,
                              static_cast<float>(windowW) - leftPanelW - rightPanelW,
                              static_cast<float>(windowH) - toolbarH);

    m_editorCamera->SetProjection(0.0f, static_cast<float>(windowW),
                                  static_cast<float>(windowH), 0.0f);
}

void EditorApp::OnWindowResize(int width, int height) {
    RecalculateLayout(width, height);
}

void EditorApp::OnToolbarAction(int action) {
    ToolbarAction ta = static_cast<ToolbarAction>(action);

    switch (ta) {
    case ToolbarAction::ADD_BUTTON: AddWidgetToScene("Button"); break;
    case ToolbarAction::ADD_LABEL: AddWidgetToScene("Label"); break;
    case ToolbarAction::ADD_IMAGE: AddWidgetToScene("Image"); break;
    case ToolbarAction::ADD_PANEL: AddWidgetToScene("Panel"); break;
    case ToolbarAction::ADD_LAYOUT: AddWidgetToScene("Layout"); break;
    case ToolbarAction::ACTION_SAVE: SaveScene(); break;
    case ToolbarAction::ACTION_LOAD: LoadScene(); break;
    case ToolbarAction::ACTION_UNDO: UndoRedoStack::GetInstance().Undo(); break;
    case ToolbarAction::ACTION_REDO: UndoRedoStack::GetInstance().Redo(); break;
    case ToolbarAction::ACTION_DELETE: DeleteSelected(); break;
    }
}

void EditorApp::AddWidgetToScene(const std::string& type) {
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
    widget->SetPosition(Vec3(0.0f, 0.0f, 0.0f));

    m_editedScene->GetRoot()->AddChild(std::move(widget));

    Logger::Info("Added widget: {}", type);
}

void EditorApp::DeleteSelected() {
    if (!m_selectedNode) return;

    Node* parent = m_selectedNode->GetParent();
    if (parent) {
        parent->RemoveChild(m_selectedNode);
        SelectNode(nullptr);
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
        SelectNode(nullptr);
        Logger::Info("Scene loaded");
    } else {
        Logger::Error("Failed to load scene");
    }
}
