#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <pybind11/stl.h>
#include <Scene.h>
#include <AssetManager.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/Sprite.h>
#include <SceneGraph/Widget.h>
#include <SceneGraph/Button.h>
#include <SceneGraph/Label.h>
#include <SceneGraph/Image.h>
#include <SceneGraph/CanvasPanel.h>
#include <SceneGraph/Layout.h>
#include <SceneGraph/UISystem.h>
#include <Render/Texture.h>
#include <Render/Renderer.h>
#include <TextRenderer.h>
#include <Timer.h>
#include <Logger.h>
#include <UI/UISerializer.h>
#include <functional>
#include <string>
#include "BindApp.h"

namespace py = pybind11;

void RegisterSceneBindings(py::module_& m)
{
    py::class_<Texture, std::shared_ptr<Texture>>(m, "Texture")
        .def("GetWidth", &Texture::GetWidth)
        .def("GetHeight", &Texture::GetHeight);

    m.def("LoadTexture", [](const std::string& path) {
        return AssetManager::GetInstance().LoadTexture(path);
    }, "Load a texture from file path (cached by AssetManager)");

    py::class_<Node>(m, "Node")
        .def(py::init<>())
        .def("SetPosition", &Node::SetPosition)
        .def("GetPosition", &Node::GetPosition,
             py::return_value_policy::reference)
        .def("SetRotationZ", &Node::SetRotationZ)
        .def("GetRotationZ", &Node::GetRotationZ)
        .def("SetScale", &Node::SetScale)
        .def("GetScale", &Node::GetScale,
             py::return_value_policy::reference)
        .def("SetVisible", &Node::SetVisible)
        .def("IsVisible", &Node::IsVisible)
        .def("SetOpacity", &Node::SetOpacity)
        .def("GetOpacity", &Node::GetOpacity)
        .def("SetContentSize", &Node::SetContentSize)
        .def("GetContentSize", &Node::GetContentSize,
             py::return_value_policy::reference)
        .def("SetName", &Node::SetName)
        .def("GetName", [](const Node& n) { return n.GetName(); })
        .def("SetZOrder", &Node::SetZOrder)
        .def("GetZOrder", &Node::GetZOrder)
        .def("SetAnchor", &Node::SetAnchor)
        .def("GetAnchor", &Node::GetAnchor,
             py::return_value_policy::reference)
        .def("SetColor", &Node::SetColor)
        .def("GetColor", &Node::GetColor,
             py::return_value_policy::reference)
        .def("GetParent", &Node::GetParent,
             py::return_value_policy::reference)
        .def("AddChild", [](Node& self, Node* child) {
            Logger::Warn("Node.add_child: ownership transfer not supported from Python; "
                         "use Scene.create_sprite instead");
            (void)self; (void)child;
        })
        .def("GetChildCount", &Node::GetChildCount)
        .def("GetChild", &Node::GetChild,
             py::return_value_policy::reference)
        .def("FindChild", &Node::FindChild,
             py::return_value_policy::reference);

    py::class_<Sprite, Node>(m, "Sprite")
        .def(py::init<>())
        .def("SetTexture", &Sprite::SetTexture)
        .def("GetTexture", &Sprite::GetTexture)
        .def("SetTexOffset", &Sprite::SetTexOffset)
        .def("SetTexScale", &Sprite::SetTexScale)
        .def("GetTexOffsetX", &Sprite::GetTexOffsetX)
        .def("GetTexOffsetY", &Sprite::GetTexOffsetY)
        .def("GetTexScaleX", &Sprite::GetTexScaleX)
        .def("GetTexScaleY", &Sprite::GetTexScaleY);

    py::class_<Widget, Node>(m, "Widget")
        .def(py::init<>())
        .def("SetEnabled", &Widget::SetEnabled)
        .def("IsEnabled", &Widget::IsEnabled)
        .def("SetTouchEnabled", &Widget::SetTouchEnabled)
        .def("IsTouchEnabled", &Widget::IsTouchEnabled)
        .def("SetFocusable", &Widget::SetFocusable)
        .def("IsFocusable", &Widget::IsFocusable)
        .def("OnTouchStarted", &Widget::OnTouchStarted)
        .def("OnTouchMoved", &Widget::OnTouchMoved)
        .def("OnTouchEnded", &Widget::OnTouchEnded)
        .def("OnKeyDown", &Widget::OnKeyDown)
        .def("OnKeyUp", &Widget::OnKeyUp);

    py::class_<Button, Widget>(m, "Button")
        .def(py::init<>())
        .def("SetNormalImage", &Button::SetNormalImage)
        .def("GetNormalImage", &Button::GetNormalImage)
        .def("SetPressedImage", &Button::SetPressedImage)
        .def("GetPressedImage", &Button::GetPressedImage)
        .def("SetDisabledImage", &Button::SetDisabledImage)
        .def("GetDisabledImage", &Button::GetDisabledImage)
        .def("SetText", &Button::SetText)
        .def("GetText", &Button::GetText)
        .def("SetTextColor", &Button::SetTextColor)
        .def("GetTextColor", &Button::GetTextColor,
             py::return_value_policy::reference)
        .def("SetFontSize", &Button::SetFontSize)
        .def("GetFontSize", &Button::GetFontSize)
        .def("SetInteractable", &Button::SetInteractable)
        .def("IsInteractable", &Button::IsInteractable)
        .def("OnClicked", &Button::OnClicked);

    py::class_<Label, Widget>(m, "Label")
        .def(py::init<>())
        .def("SetText", &Label::SetText)
        .def("GetText", &Label::GetText)
        .def("SetFontSize", &Label::SetFontSize)
        .def("GetFontSize", &Label::GetFontSize)
        .def("SetTextColor", &Label::SetTextColor)
        .def("GetTextColor", &Label::GetTextColor,
             py::return_value_policy::reference)
        .def("SetBackground", &Label::SetBackground)
        .def("GetBackground", &Label::GetBackground)
        .def("SetHAlign", &Label::SetHAlign)
        .def("GetHAlign", &Label::GetHAlign)
        .def("SetVAlign", &Label::SetVAlign)
        .def("GetVAlign", &Label::GetVAlign)
        .def("SetLineSpacing", &Label::SetLineSpacing)
        .def("GetLineSpacing", &Label::GetLineSpacing);

    py::class_<CanvasPanel, Widget>(m, "CanvasPanel")
        .def(py::init<>())
        .def("UpdateLayout", &CanvasPanel::UpdateLayout);

    py::class_<Layout, Widget>(m, "Layout")
        .def(py::init<>())
        .def("SetLayoutType", &Layout::SetLayoutType)
        .def("GetLayoutType", &Layout::GetLayoutType)
        .def("SetSpacing", &Layout::SetSpacing)
        .def("GetSpacing", &Layout::GetSpacing)
        .def("SetPadding", &Layout::SetPadding)
        .def("GetPadding", &Layout::GetPadding,
             py::return_value_policy::reference)
        .def("SetGridColumns", &Layout::SetGridColumns)
        .def("GetGridColumns", &Layout::GetGridColumns)
        .def("DoLayout", &Layout::DoLayout);

    py::class_<Image, Sprite>(m, "Image")
        .def(py::init<>())
        .def("SetScale9Enabled", &Image::SetScale9Enabled)
        .def("IsScale9Enabled", &Image::IsScale9Enabled)
        .def("SetCapInsets", &Image::SetCapInsets)
        .def("GetCapInsets", &Image::GetCapInsets,
             py::return_value_policy::reference);

    m.def("LoadUI", [](const std::string& filepath) -> Node* {
        return UISerializer::LoadFromFile(filepath);
    }, "Load a .cui UI file and return the deserialized Node tree",
       py::return_value_policy::reference);

    // === E1: 动态创建 UI 控件并挂载到任意已有节点 ===
    // 生命周期由 C++ 侧持有（随父节点销毁）；Python 侧只持引用。
    // 挂到 Layout 下后需调用 Layout.DoLayout() 生效。parent 无效时返回 None。

    m.def("CreateButton", [](Node* parent, const std::string& name) -> Button* {
        if (!parent) return nullptr;
        auto widget = std::make_unique<Button>();
        widget->SetName(name);
        Button* raw = widget.get();
        parent->AddChild(std::move(widget));
        return raw;
    }, py::arg("parent"), py::arg("name"),
       "Create a Button attached to parent", py::return_value_policy::reference);

    m.def("CreateLabel", [](Node* parent, const std::string& name) -> Label* {
        if (!parent) return nullptr;
        auto widget = std::make_unique<Label>();
        widget->SetName(name);
        Label* raw = widget.get();
        parent->AddChild(std::move(widget));
        return raw;
    }, py::arg("parent"), py::arg("name"),
       "Create a Label attached to parent", py::return_value_policy::reference);

    m.def("CreateImage", [](Node* parent, const std::string& name) -> Image* {
        if (!parent) return nullptr;
        auto widget = std::make_unique<Image>();
        widget->SetName(name);
        Image* raw = widget.get();
        parent->AddChild(std::move(widget));
        return raw;
    }, py::arg("parent"), py::arg("name"),
       "Create an Image attached to parent", py::return_value_policy::reference);

    m.def("CreateSprite", [](Node* parent, const std::string& name) -> Sprite* {
        if (!parent) return nullptr;
        auto widget = std::make_unique<Sprite>();
        widget->SetName(name);
        Sprite* raw = widget.get();
        parent->AddChild(std::move(widget));
        return raw;
    }, py::arg("parent"), py::arg("name"),
       "Create a Sprite attached to parent", py::return_value_policy::reference);

    m.def("CreateCanvasPanel", [](Node* parent, const std::string& name) -> CanvasPanel* {
        if (!parent) return nullptr;
        auto widget = std::make_unique<CanvasPanel>();
        widget->SetName(name);
        CanvasPanel* raw = widget.get();
        parent->AddChild(std::move(widget));
        return raw;
    }, py::arg("parent"), py::arg("name"),
       "Create a CanvasPanel attached to parent", py::return_value_policy::reference);

    m.def("CreateLayout", [](Node* parent, const std::string& name) -> Layout* {
        if (!parent) return nullptr;
        auto widget = std::make_unique<Layout>();
        widget->SetName(name);
        Layout* raw = widget.get();
        parent->AddChild(std::move(widget));
        return raw;
    }, py::arg("parent"), py::arg("name"),
       "Create a Layout attached to parent", py::return_value_policy::reference);

    py::enum_<TextRenderer::Align>(m, "TextAlign")
        .value("LEFT", TextRenderer::Align::Left)
        .value("CENTER", TextRenderer::Align::Center)
        .value("RIGHT", TextRenderer::Align::Right);

    py::enum_<TextRenderer::VAlign>(m, "VAlign")
        .value("TOP", TextRenderer::VAlign::Top)
        .value("MIDDLE", TextRenderer::VAlign::Middle)
        .value("BOTTOM", TextRenderer::VAlign::Bottom);

    m.def("WrapText", [](const std::string& text, float maxWidth, float scale) {
        TextRenderer* tr = UISystem::GetInstance().GetFontRenderer();
        if (!tr) return std::vector<std::string>();
        return tr->WrapString(text, maxWidth, scale);
    }, py::arg("text"), py::arg("max_width"), py::arg("scale") = 1.0f,
       "Wrap text to lines by max width (font scale), returns a list of lines");

    m.def("MeasureText", [](const std::string& text, float scale) {
        TextRenderer* tr = UISystem::GetInstance().GetFontRenderer();
        if (!tr) return Vec2(0.0f, 0.0f);
        return tr->MeasureString(text, scale);
    }, py::arg("text"), py::arg("scale") = 1.0f,
       "Measure text size in pixels (font scale)");

    py::enum_<Layout::Type>(m, "LayoutType")
        .value("VERTICAL", Layout::Type::VERTICAL)
        .value("HORIZONTAL", Layout::Type::HORIZONTAL)
        .value("GRID", Layout::Type::GRID);

    py::class_<UISystem>(m, "UISystem")
        .def_static("GetInstance", &UISystem::GetInstance,
                     py::return_value_policy::reference)
        .def("ProcessEvents", &UISystem::ProcessEvents)
        .def("AddUI", &UISystem::AddUI,
             py::arg("filepath"), py::arg("zorder") = 0, py::arg("modal") = false,
             py::return_value_policy::reference,
             "Load a .cui and mount it as an overlay layer (zorder ascending); "
             "modal=true blocks hit-testing to lower layers when the layer misses. "
             "Returns the layer container (None on failure)")
        .def("RemoveUI", &UISystem::RemoveUI,
             "Detach a layer; the Python reference becomes dangling after this")
        .def("GetLayers", &UISystem::GetLayers,
             py::return_value_policy::reference)
        .def("GetPressedWidget", &UISystem::GetPressedWidget,
             py::return_value_policy::reference)
        .def("GetHoveredWidget", &UISystem::GetHoveredWidget,
             py::return_value_policy::reference)
        .def("GetFocusedWidget", &UISystem::GetFocusedWidget,
             py::return_value_policy::reference);

    py::class_<Scene, std::shared_ptr<Scene>>(m, "Scene")
        .def(py::init<>())
        .def("CreateSprite", [](Scene& self,
                                  const std::string& name,
                                  std::shared_ptr<Texture> texture,
                                  float x, float y,
                                  float w, float h) -> Sprite* {
            float color[4] = {1.0f, 1.0f, 1.0f, 1.0f};
            Sprite* sprite = self.CreateSprite(
                Vec3(x, y, 0.0f), Vec2(w, h), texture, color);
            if (sprite) sprite->SetName(name);
            return sprite;
        }, py::arg("name"), py::arg("texture"),
           py::arg("x") = 0.0f, py::arg("y") = 0.0f,
           py::arg("w") = 64.0f, py::arg("h") = 64.0f,
           py::return_value_policy::reference)
        .def("OnUpdate", &Scene::OnUpdate)
        .def("OnRender", &Scene::OnRender)
        .def("GetRoot", &Scene::GetRoot,
             py::return_value_policy::reference)
        .def("RemoveAllChildren", &Scene::RemoveAllChildren);

    py::class_<SceneManager>(m, "SceneManager")
        .def_static("GetInstance", &SceneManager::GetInstance,
                     py::return_value_policy::reference)
        .def("PushScene", [](SceneManager& self, std::shared_ptr<Scene> scene) {
            self.PushScene(std::move(scene));
        })
        .def("PopScene", &SceneManager::PopScene)
        .def("GetCurrentScene", &SceneManager::GetCurrentScene,
             py::return_value_policy::reference);

}
