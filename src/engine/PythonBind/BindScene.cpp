#include <pybind11/pybind11.h>
#include <Scene.h>
#include <AssetManager.h>
#include <SceneGraph/Node.h>
#include <SceneGraph/Sprite.h>
#include <Render/Texture.h>
#include <Render/Renderer.h>
#include <Timer.h>
#include <Logger.h>
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
