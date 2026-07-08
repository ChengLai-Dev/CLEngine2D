#include <pybind11/pybind11.h>
#include <pybind11/functional.h>
#include <Input/InputAction.h>
#include <Input/InputMappingContext.h>
#include <Input/InputSystem.h>

namespace py = pybind11;

void RegisterInputActionsBindings(py::module_& m)
{
    py::enum_<EInputActionAccumulationBehavior>(m, "EInputActionAccumulationBehavior")
        .value("TakeHighestAbsoluteValue", EInputActionAccumulationBehavior::TakeHighestAbsoluteValue)
        .value("Cumulative", EInputActionAccumulationBehavior::Cumulative);

    py::enum_<EInputMode>(m, "EInputMode")
        .value("GameOnly", EInputMode::GameOnly)
        .value("UIOnly", EInputMode::UIOnly)
        .value("GameAndUI", EInputMode::GameAndUI);

    py::class_<InputActionValue>(m, "InputActionValue")
        .def(py::init<>())
        .def(py::init<float, float>(), py::arg("x") = 0.0f, py::arg("y") = 0.0f)
        .def_readwrite("x", &InputActionValue::x)
        .def_readwrite("y", &InputActionValue::y)
        .def("IsZero", &InputActionValue::IsZero)
        .def("GetVec2", &InputActionValue::GetVec2)
        .def("GetAxis", &InputActionValue::GetAxis)
        .def("__repr__", [](const InputActionValue& v) {
            return "<InputActionValue(" + std::to_string(v.x) + ", " + std::to_string(v.y) + ")>";
        });

    py::class_<InputAction, std::shared_ptr<InputAction>>(m, "InputAction")
        .def(py::init<>())
        .def("OnStarted", &InputAction::OnStarted, py::keep_alive<1, 2>())
        .def("OnTriggered", &InputAction::OnTriggered, py::keep_alive<1, 2>())
        .def("OnCompleted", &InputAction::OnCompleted, py::keep_alive<1, 2>())
        .def("GetValue", &InputAction::GetValue, py::return_value_policy::reference)
        .def("IsActive", &InputAction::IsActive)
        .def_readwrite("bConsumeInput", &InputAction::bConsumeInput)
        .def_readwrite("AccumulationBehavior", &InputAction::AccumulationBehavior);

    py::class_<InputMappingContext, std::shared_ptr<InputMappingContext>>(m, "InputMappingContext")
        .def(py::init<>())
        .def("MapKey", &InputMappingContext::MapKey,
             py::arg("action"), py::arg("key"),
             py::arg("scale") = Vec2(1.0f, 0.0f))
        .def("MapMouse", &InputMappingContext::MapMouse,
             py::arg("action"), py::arg("button"),
             py::arg("scale") = Vec2(1.0f, 0.0f))
        .def_readwrite("InputMode", &InputMappingContext::InputMode);

    py::class_<InputSystem>(m, "InputSystem")
        .def_static("GetInstance", &InputSystem::GetInstance,
                     py::return_value_policy::reference)
        .def("AddContext", &InputSystem::AddContext,
             py::arg("context"), py::arg("priority") = 0)
        .def("RemoveContext", &InputSystem::RemoveContext)
        .def("SetInputMode", &InputSystem::SetInputMode)
        .def("GetInputMode", &InputSystem::GetInputMode)
        .def("PollEvents", &InputSystem::PollEvents)
        .def("Advance", &InputSystem::Advance)
        .def("Clear", &InputSystem::Clear);
}
