#include <pybind11/embed.h>

namespace py = pybind11;

// Forward declarations of registration functions
void RegisterMathBindings(py::module_& m);
void RegisterInputBindings(py::module_& m);
void RegisterInputActionsBindings(py::module_& m);
void RegisterRendererBindings(py::module_& m);
void RegisterSceneBindings(py::module_& m);

PYBIND11_EMBEDDED_MODULE(clengine, m)
{
    m.doc() = "CLEngine2D Python bindings";

    RegisterMathBindings(m);
    RegisterInputBindings(m);
    RegisterInputActionsBindings(m);
    RegisterRendererBindings(m);
    RegisterSceneBindings(m);
}
