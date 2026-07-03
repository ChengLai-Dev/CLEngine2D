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
        .def("get_width", &Texture::GetWidth)
        .def("get_height", &Texture::GetHeight);

    m.def("load_texture", [](const std::string& path) {
        return AssetManager::GetInstance().LoadTexture(path);
    }, "Load a texture from file path (cached by AssetManager)");

    py::class_<Node>(m, "Node")
        .def(py::init<>())
        .def("set_position", &Node::SetPosition)
        .def("get_position", &Node::GetPosition,
             py::return_value_policy::reference)
        .def("set_rotation_z", &Node::SetRotationZ)
        .def("get_rotation_z", &Node::GetRotationZ)
        .def("set_scale", &Node::SetScale)
        .def("get_scale", &Node::GetScale,
             py::return_value_policy::reference)
        .def("set_visible", &Node::SetVisible)
        .def("is_visible", &Node::IsVisible)
        .def("set_opacity", &Node::SetOpacity)
        .def("get_opacity", &Node::GetOpacity)
        .def("set_content_size", &Node::SetContentSize)
        .def("get_content_size", &Node::GetContentSize,
             py::return_value_policy::reference)
        .def("set_name", &Node::SetName)
        .def("get_name", [](const Node& n) { return n.GetName(); })
        .def("add_child", [](Node& self, Node* child) {
            Logger::Warn("Node.add_child: ownership transfer not supported from Python; "
                         "use Scene.create_sprite instead");
            (void)self; (void)child;
        })
        .def("get_child_count", &Node::GetChildCount)
        .def("get_child", &Node::GetChild,
             py::return_value_policy::reference)
        .def("find_child", &Node::FindChild,
             py::return_value_policy::reference);

    py::class_<Sprite, Node>(m, "Sprite")
        .def(py::init<>())
        .def("set_texture", &Sprite::SetTexture)
        .def("get_texture", &Sprite::GetTexture)
        .def("set_tex_offset", &Sprite::SetTexOffset)
        .def("set_tex_scale", &Sprite::SetTexScale)
        .def("get_tex_offset_x", &Sprite::GetTexOffsetX)
        .def("get_tex_offset_y", &Sprite::GetTexOffsetY)
        .def("get_tex_scale_x", &Sprite::GetTexScaleX)
        .def("get_tex_scale_y", &Sprite::GetTexScaleY);

    py::class_<Scene, std::shared_ptr<Scene>>(m, "Scene")
        .def(py::init<>())
        .def("create_sprite", [](Scene& self,
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
        .def("on_update", &Scene::OnUpdate)
        .def("on_render", &Scene::OnRender)
        .def("get_root", &Scene::GetRoot,
             py::return_value_policy::reference)
        .def("remove_all_children", &Scene::RemoveAllChildren);

    py::class_<SceneManager>(m, "SceneManager")
        .def_static("get_instance", &SceneManager::GetInstance,
                     py::return_value_policy::reference)
        .def("push_scene", [](SceneManager& self, std::shared_ptr<Scene> scene) {
            self.PushScene(std::move(scene));
        })
        .def("pop_scene", &SceneManager::PopScene)
        .def("get_current_scene", &SceneManager::GetCurrentScene,
             py::return_value_policy::reference);

    m.def("get_delta_time", []() -> float {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetDeltaTime() : 0.0f;
    });

    m.def("get_fps", []() -> float {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetFPS() : 0.0f;
    });
}
