#include <pybind11/pybind11.h>
#include <Render/Renderer.h>
#include <Render/RenderCommand.h>
#include <Render/OrthographicCamera.h>
#include <Render/Texture.h>
#include <Math/Vec3.h>
#include "BindApp.h"

namespace py = pybind11;

void RegisterRendererBindings(py::module_& m)
{
    py::class_<OrthographicCamera>(m, "OrthographicCamera")
        .def(py::init<float, float, float, float>(),
             "Create orthographic camera (left, right, bottom, top)")
        .def("set_position", &OrthographicCamera::SetPosition)
        .def("get_position", &OrthographicCamera::GetPosition,
             py::return_value_policy::reference)
        .def("set_rotation", &OrthographicCamera::SetRotation)
        .def("get_rotation", &OrthographicCamera::GetRotation);

    py::class_<Renderer>(m, "Renderer")
        .def(py::init<unsigned int>(), py::arg("quad_capacity") = 1000)
        .def("begin_scene", &Renderer::BeginScene)
        .def("end_scene", &Renderer::EndScene)
        .def("draw_quad", [](Renderer& r,
                              float x, float y, float w, float h,
                              float r_, float g_, float b_, float a_) {
            float color[4] = {r_, g_, b_, a_};
            r.DrawQuad(Vec3(x, y, 0.0f), Vec3(w, h, 1.0f), 0.0f,
                       nullptr, color);
        }, "Draw a colored quad at (x,y) with size (w,h)")
        .def("draw_quad_textured", [](Renderer& r,
                                       float x, float y, float w, float h,
                                       Texture* tex,
                                       float r_, float g_, float b_, float a_) {
            float color[4] = {r_, g_, b_, a_};
            r.DrawQuad(Vec3(x, y, 0.0f), Vec3(w, h, 1.0f), 0.0f,
                       tex, color);
        }, "Draw a textured quad at (x,y) with size (w,h)");

    m.def("set_clear_color", [](float r, float g, float b, float a) {
        RenderCommand::SetClearColor(r, g, b, a);
    });

    m.def("get_renderer", []() -> Renderer* {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetRenderer() : nullptr;
    }, py::return_value_policy::reference);

    m.def("get_game_camera", []() -> OrthographicCamera* {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetGameCamera() : nullptr;
    }, py::return_value_policy::reference);
}
