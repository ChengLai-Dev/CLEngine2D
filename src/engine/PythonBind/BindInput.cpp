#include <pybind11/pybind11.h>
#include <Input/InputCodes.h>
#include <Input/RawInput.h>

namespace py = pybind11;

void RegisterInputBindings(py::module_& m)
{
    py::enum_<KeyCode>(m, "KeyCode")
        .value("Unknown", KeyCode::Unknown)
        .value("Space", KeyCode::Space)
        .value("Apostrophe", KeyCode::Apostrophe)
        .value("Comma", KeyCode::Comma)
        .value("Minus", KeyCode::Minus)
        .value("Period", KeyCode::Period)
        .value("Slash", KeyCode::Slash)
        .value("Number0", KeyCode::Number0)
        .value("Number1", KeyCode::Number1)
        .value("Number2", KeyCode::Number2)
        .value("Number3", KeyCode::Number3)
        .value("Number4", KeyCode::Number4)
        .value("Number5", KeyCode::Number5)
        .value("Number6", KeyCode::Number6)
        .value("Number7", KeyCode::Number7)
        .value("Number8", KeyCode::Number8)
        .value("Number9", KeyCode::Number9)
        .value("Semicolon", KeyCode::Semicolon)
        .value("Equal", KeyCode::Equal)
        .value("A", KeyCode::A).value("B", KeyCode::B)
        .value("C", KeyCode::C).value("D", KeyCode::D)
        .value("E", KeyCode::E).value("F", KeyCode::F)
        .value("G", KeyCode::G).value("H", KeyCode::H)
        .value("I", KeyCode::I).value("J", KeyCode::J)
        .value("K", KeyCode::K).value("L", KeyCode::L)
        .value("M", KeyCode::M).value("N", KeyCode::N)
        .value("O", KeyCode::O).value("P", KeyCode::P)
        .value("Q", KeyCode::Q).value("R", KeyCode::R)
        .value("S", KeyCode::S).value("T", KeyCode::T)
        .value("U", KeyCode::U).value("V", KeyCode::V)
        .value("W", KeyCode::W).value("X", KeyCode::X)
        .value("Y", KeyCode::Y).value("Z", KeyCode::Z)
        .value("Escape", KeyCode::Escape)
        .value("Enter", KeyCode::Enter)
        .value("Tab", KeyCode::Tab)
        .value("Backspace", KeyCode::Backspace)
        .value("Insert", KeyCode::Insert)
        .value("Delete", KeyCode::Delete)
        .value("Right", KeyCode::Right)
        .value("Left", KeyCode::Left)
        .value("Down", KeyCode::Down)
        .value("Up", KeyCode::Up)
        .value("LeftShift", KeyCode::LeftShift)
        .value("LeftControl", KeyCode::LeftControl)
        .value("LeftAlt", KeyCode::LeftAlt)
        .value("RightShift", KeyCode::RightShift)
        .value("RightControl", KeyCode::RightControl)
        .value("RightAlt", KeyCode::RightAlt)
        .value("F1", KeyCode::F1).value("F2", KeyCode::F2)
        .value("F3", KeyCode::F3).value("F4", KeyCode::F4)
        .value("F5", KeyCode::F5);

    py::enum_<MouseCode>(m, "MouseCode")
        .value("ButtonLeft", MouseCode::ButtonLeft)
        .value("ButtonRight", MouseCode::ButtonRight)
        .value("ButtonMiddle", MouseCode::ButtonMiddle);

    m.def("IsKeyDown", &RawInput::IsKeyDown);
    m.def("IsKeyPressed", &RawInput::IsKeyPressed);
    m.def("IsKeyReleased", &RawInput::IsKeyReleased);

    m.def("IsMouseButtonDown", &RawInput::IsMouseButtonDown);
    m.def("IsMouseButtonPressed", &RawInput::IsMouseButtonPressed);
    m.def("IsMouseButtonReleased", &RawInput::IsMouseButtonReleased);
    m.def("GetMouseX", &RawInput::GetMouseX);
    m.def("GetMouseY", &RawInput::GetMouseY);
    m.def("GetMousePosition", &RawInput::GetMousePosition);
}
