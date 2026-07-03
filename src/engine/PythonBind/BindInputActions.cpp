#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <Input/InputAction.h>
#include <Input/InputMappingContext.h>
#include <Input/InputSystem.h>

namespace py = pybind11;

void RegisterInputActionsBindings(py::module_& m)
{
    py::class_<InputActionValue>(m, "InputActionValue")
        .def(py::init<>())
        .def(py::init<float, float>(), py::arg("x") = 0.0f, py::arg("y") = 0.0f)
        .def_readwrite("x", &InputActionValue::x)
        .def_readwrite("y", &InputActionValue::y)
        .def("is_zero", &InputActionValue::IsZero)
        .def("get_vec2", &InputActionValue::GetVec2)
        .def("get_axis", &InputActionValue::GetAxis)
        .def("__repr__", [](const InputActionValue& v) {
            return "<InputActionValue(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")>";
        });

    py::class_<InputAction, std::shared_ptr<InputAction>>(m, "InputAction")
        .def(py::init<>())
        .def("on_started", &InputAction::OnStarted)
        .def("on_triggered", &InputAction::OnTriggered)
        .def("on_completed", &InputAction::OnCompleted)
        .def("get_value", &InputAction::GetValue, py::return_value_policy::reference)
        .def("is_active", &InputAction::IsActive);

    py::class_<InputMappingContext, std::shared_ptr<InputMappingContext>>(m, "InputMappingContext")
        .def(py::init<>())
        .def("map_key", &InputMappingContext::MapKey,
             py::arg("action"), py::arg("key"),
             py::arg("scale") = Vec2(1.0f, 0.0f),
             py::keep_alive<1, 2>())
        .def("map_mouse", &InputMappingContext::MapMouse,
             py::arg("action"), py::arg("button"),
             py::arg("scale") = Vec2(1.0f, 0.0f),
             py::keep_alive<1, 2>());

    py::class_<InputSystem>(m, "InputSystem")
        .def_static("get_instance", &InputSystem::GetInstance,
                     py::return_value_policy::reference)
        .def("add_context", &InputSystem::AddContext,
             py::arg("context"), py::arg("priority") = 0)
        .def("remove_context", &InputSystem::RemoveContext)
        .def("update", &InputSystem::Update)
        .def("clear", &InputSystem::Clear);
}
