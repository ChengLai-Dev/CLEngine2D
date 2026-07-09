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
        .def("SetPosition", &OrthographicCamera::SetPosition)
        .def("GetPosition", &OrthographicCamera::GetPosition,
             py::return_value_policy::reference)
        .def("SetRotation", &OrthographicCamera::SetRotation)
        .def("GetRotation", &OrthographicCamera::GetRotation);

    py::class_<Renderer>(m, "Renderer")
        .def(py::init<unsigned int>(), py::arg("quad_capacity") = 1000)
        .def("BeginScene", &Renderer::BeginScene)
        .def("EndScene", &Renderer::EndScene)
        .def("DrawQuad", [](Renderer& r,
                              float x, float y, float w, float h,
                              float r_, float g_, float b_, float a_) {
            r.DrawQuad(Vec3(x, y, 0.0f), Vec3(w, h, 1.0f), 0.0f,
                       Color(r_, g_, b_, a_));
        }, "Draw a colored quad at (x,y) with size (w,h)")
        .def("DrawQuadTextured", [](Renderer& r,
                                       float x, float y, float w, float h,
                                       Texture* tex,
                                       float r_, float g_, float b_, float a_) {
            r.DrawQuad(Vec3(x, y, 0.0f), Vec3(w, h, 1.0f), 0.0f,
                       Color(r_, g_, b_, a_), tex);
        }, "Draw a textured quad at (x,y) with size (w,h)")
        .def("DrawLine", [](Renderer& r,
                              float x1, float y1, float x2, float y2,
                              float r_, float g_, float b_, float a_) {
            r.DrawLine(Vec3(x1, y1, 0.0f), Vec3(x2, y2, 0.0f),
                       Color(r_, g_, b_, a_));
        }, "Draw a line from (x1,y1) to (x2,y2)");

    m.def("SetClearColor", [](float r, float g, float b, float a) {
        RenderCommand::SetClearColor(r, g, b, a);
    });

    m.def("GetRenderer", []() -> Renderer* {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetRenderer() : nullptr;
    }, py::return_value_policy::reference);

    m.def("GetGameCamera", []() -> OrthographicCamera* {
        PythonScriptApp* app = PythonScriptApp::GetCurrent();
        return app ? app->GetGameCamera() : nullptr;
    }, py::return_value_policy::reference);
}
