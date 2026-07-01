#include "Input.h"
#include <GLFW/glfw3.h>

Input::InputState Input::s_state;
bool Input::s_gamepadConnected[Input::GAMEPAD_COUNT] = {};

bool Input::IsKeyDown(KeyCode key) {
    auto idx = static_cast<uint16_t>(key);
    if (idx >= KEY_COUNT) return false;
    auto state = s_state.keyStates[idx];
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool Input::IsKeyPressed(KeyCode key) {
    auto idx = static_cast<uint16_t>(key);
    if (idx >= KEY_COUNT) return false;
    return s_state.keyStates[idx] == KeyState::Pressed;
}

bool Input::IsKeyReleased(KeyCode key) {
    auto idx = static_cast<uint16_t>(key);
    if (idx >= KEY_COUNT) return false;
    return s_state.keyStates[idx] == KeyState::Released;
}

bool Input::IsMouseButtonDown(MouseCode button) {
    auto idx = static_cast<uint16_t>(button);
    if (idx >= MOUSE_COUNT) return false;
    auto state = s_state.mouseStates[idx];
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool Input::IsMouseButtonPressed(MouseCode button) {
    auto idx = static_cast<uint16_t>(button);
    if (idx >= MOUSE_COUNT) return false;
    return s_state.mouseStates[idx] == KeyState::Pressed;
}

bool Input::IsMouseButtonReleased(MouseCode button) {
    auto idx = static_cast<uint16_t>(button);
    if (idx >= MOUSE_COUNT) return false;
    return s_state.mouseStates[idx] == KeyState::Released;
}

float Input::GetMouseX() {
    return static_cast<float>(s_state.mouseX);
}

float Input::GetMouseY() {
    return static_cast<float>(s_state.mouseY);
}

std::pair<float, float> Input::GetMousePosition() {
    return { static_cast<float>(s_state.mouseX), static_cast<float>(s_state.mouseY) };
}

float Input::GetScrollDeltaX() {
    return static_cast<float>(s_state.scrollX);
}

float Input::GetScrollDeltaY() {
    return static_cast<float>(s_state.scrollY);
}

bool Input::IsGamepadConnected(int gamepadIndex) {
    if (gamepadIndex < 0 || gamepadIndex >= GAMEPAD_COUNT) return false;
    return s_gamepadConnected[gamepadIndex];
}

float Input::GetGamepadAxis(GamepadAxis axis, int gamepadIndex) {
    if (gamepadIndex < 0 || gamepadIndex >= GAMEPAD_COUNT) return false;
    if (!s_gamepadConnected[gamepadIndex]) return 0.0f;

    GLFWgamepadstate state;
    if (glfwGetGamepadState(gamepadIndex, &state)) {
        int axisIdx = static_cast<int>(axis);
        if (axisIdx >= 0 && axisIdx < GAMEPAD_AXIS_COUNT) {
            return state.axes[axisIdx];
        }
    }
    return 0.0f;
}

bool Input::IsGamepadButtonDown(GamepadButton button, int gamepadIndex) {
    if (gamepadIndex < 0 || gamepadIndex >= GAMEPAD_COUNT) return false;
    if (!s_gamepadConnected[gamepadIndex]) return false;

    GLFWgamepadstate state;
    if (glfwGetGamepadState(gamepadIndex, &state)) {
        int btnIdx = static_cast<int>(button);
        return btnIdx >= 0 && btnIdx < GAMEPAD_BUTTON_COUNT && state.buttons[btnIdx] == GLFW_PRESS;
    }
    return false;
}

bool Input::IsGamepadButtonPressed(GamepadButton button, int gamepadIndex) {
    return IsGamepadButtonDown(button, gamepadIndex);
}

void Input::OnKeyEvent(int glfwKey, int action) {
    if (glfwKey < 0 || glfwKey >= KEY_COUNT) return;

    switch (action) {
        case GLFW_PRESS:
            if (s_state.keyStates[glfwKey] == KeyState::None ||
                s_state.keyStates[glfwKey] == KeyState::Released) {
                s_state.keyStates[glfwKey] = KeyState::Pressed;
            }
            break;
        case GLFW_RELEASE:
            if (s_state.keyStates[glfwKey] == KeyState::Pressed ||
                s_state.keyStates[glfwKey] == KeyState::Held) {
                s_state.keyStates[glfwKey] = KeyState::Released;
            }
            break;
        default:
            break;
    }
}

void Input::OnMouseButtonEvent(int glfwButton, int action) {
    if (glfwButton < 0 || glfwButton >= MOUSE_COUNT) return;

    switch (action) {
        case GLFW_PRESS:
            if (s_state.mouseStates[glfwButton] == KeyState::None ||
                s_state.mouseStates[glfwButton] == KeyState::Released) {
                s_state.mouseStates[glfwButton] = KeyState::Pressed;
            }
            break;
        case GLFW_RELEASE:
            if (s_state.mouseStates[glfwButton] == KeyState::Pressed ||
                s_state.mouseStates[glfwButton] == KeyState::Held) {
                s_state.mouseStates[glfwButton] = KeyState::Released;
            }
            break;
        default:
            break;
    }
}

void Input::OnMouseMoveEvent(double x, double y) {
    s_state.mouseX = x;
    s_state.mouseY = y;
}

void Input::OnScrollEvent(double xOffset, double yOffset) {
    s_state.scrollX += xOffset;
    s_state.scrollY += yOffset;
}

void Input::Update() {
    for (int i = 0; i < KEY_COUNT; ++i) {
        switch (s_state.keyStates[i]) {
            case KeyState::Pressed:
                s_state.keyStates[i] = KeyState::Held;
                break;
            case KeyState::Released:
                s_state.keyStates[i] = KeyState::None;
                break;
            default:
                break;
        }
    }

    for (int i = 0; i < MOUSE_COUNT; ++i) {
        switch (s_state.mouseStates[i]) {
            case KeyState::Pressed:
                s_state.mouseStates[i] = KeyState::Held;
                break;
            case KeyState::Released:
                s_state.mouseStates[i] = KeyState::None;
                break;
            default:
                break;
        }
    }

    s_state.scrollX = 0.0;
    s_state.scrollY = 0.0;

    for (int i = 0; i < GAMEPAD_COUNT; ++i) {
        s_gamepadConnected[i] = (glfwJoystickPresent(GLFW_JOYSTICK_1 + i) &&
                                 glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i));
    }
}
