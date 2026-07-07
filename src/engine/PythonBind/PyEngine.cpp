#include <pybind11/embed.h>
#include "BindApp.h"

namespace py = pybind11;

// Forward declarations of registration functions
void RegisterMathBindings(py::module_& m);
void RegisterInputBindings(py::module_& m);
void RegisterInputActionsBindings(py::module_& m);
void RegisterRendererBindings(py::module_& m);
void RegisterSceneBindings(py::module_& m);

PYBIND11_EMBEDDED_MODULE(CLEngine, m)
{
    m.doc() = "CLEngine2D Python bindings";

    // Engine-level utility functions on root module
    m.def("GetDeltaTime", []() -> float {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetDeltaTime() : 0.0f;
    });

    m.def("GetFPS", []() -> float {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetFPS() : 0.0f;
    });

    m.def("ReloadScripts", []() {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        if (app) app->ReloadScripts();
    });

    // Sub-modules
    auto math = m.def_submodule("Math", "Math types: Vec2, Vec3, Mat4");
    RegisterMathBindings(math);

    auto input = m.def_submodule("Input", "Key codes, mouse codes, raw input, input action system");
    RegisterInputBindings(input);
    RegisterInputActionsBindings(input);

    auto renderer = m.def_submodule("Renderer", "Camera, Renderer, draw commands");
    RegisterRendererBindings(renderer);

    auto sceneGraph = m.def_submodule("SceneGraph", "Scene graph: Node, Sprite, Scene, SceneManager, textures");
    RegisterSceneBindings(sceneGraph);
}
