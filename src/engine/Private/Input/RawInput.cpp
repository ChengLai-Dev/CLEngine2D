#include "Input/RawInput.h"
#include <GLFW/glfw3.h>
#define WIN32_LEAN_AND_MEAN
#include <windows.h>

RawInput::InputState RawInput::s_state;
bool RawInput::s_gamepadConnected[RawInput::GAMEPAD_COUNT] = {};
std::string RawInput::s_charBuffer;

bool RawInput::IsKeyDown(KeyCode key) {
    uint16_t idx = static_cast<uint16_t>(key);
    if (idx >= KEY_COUNT) return false;
    KeyState state = s_state.keyStates[idx];
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool RawInput::IsKeyPressed(KeyCode key) {
    uint16_t idx = static_cast<uint16_t>(key);
    if (idx >= KEY_COUNT) return false;
    return s_state.keyStates[idx] == KeyState::Pressed;
}

bool RawInput::IsKeyReleased(KeyCode key) {
    uint16_t idx = static_cast<uint16_t>(key);
    if (idx >= KEY_COUNT) return false;
    return s_state.keyStates[idx] == KeyState::Released;
}

bool RawInput::IsMouseButtonDown(MouseCode button) {
    uint16_t idx = static_cast<uint16_t>(button);
    if (idx >= MOUSE_COUNT) return false;
    KeyState state = s_state.mouseStates[idx];
    return state == KeyState::Pressed || state == KeyState::Held;
}

bool RawInput::IsMouseButtonPressed(MouseCode button) {
    uint16_t idx = static_cast<uint16_t>(button);
    if (idx >= MOUSE_COUNT) return false;
    return s_state.mouseStates[idx] == KeyState::Pressed;
}

bool RawInput::IsMouseButtonReleased(MouseCode button) {
    uint16_t idx = static_cast<uint16_t>(button);
    if (idx >= MOUSE_COUNT) return false;
    return s_state.mouseStates[idx] == KeyState::Released;
}

float RawInput::GetMouseX() {
    return static_cast<float>(s_state.mouseX);
}

float RawInput::GetMouseY() {
    return static_cast<float>(s_state.mouseY);
}

Vec2 RawInput::GetMousePosition() {
    return Vec2(static_cast<float>(s_state.mouseX), static_cast<float>(s_state.mouseY));
}

float RawInput::GetScrollDeltaX() {
    return static_cast<float>(s_state.scrollX);
}

float RawInput::GetScrollDeltaY() {
    return static_cast<float>(s_state.scrollY);
}

bool RawInput::IsGamepadConnected(int gamepadIndex) {
    if (gamepadIndex < 0 || gamepadIndex >= GAMEPAD_COUNT) return false;
    return s_gamepadConnected[gamepadIndex];
}

float RawInput::GetGamepadAxis(GamepadAxis axis, int gamepadIndex) {
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

bool RawInput::IsGamepadButtonDown(GamepadButton button, int gamepadIndex) {
    if (gamepadIndex < 0 || gamepadIndex >= GAMEPAD_COUNT) return false;
    if (!s_gamepadConnected[gamepadIndex]) return false;

    GLFWgamepadstate state;
    if (glfwGetGamepadState(gamepadIndex, &state)) {
        int btnIdx = static_cast<int>(button);
        return btnIdx >= 0 && btnIdx < GAMEPAD_BUTTON_COUNT && state.buttons[btnIdx] == GLFW_PRESS;
    }
    return false;
}

bool RawInput::IsGamepadButtonPressed(GamepadButton button, int gamepadIndex) {
    return IsGamepadButtonDown(button, gamepadIndex);
}

static std::string CodepointToUTF8(unsigned int codepoint) {
    std::string result;
    if (codepoint <= 0x7F) {
        result += static_cast<char>(codepoint);
    } else if (codepoint <= 0x7FF) {
        result += static_cast<char>(0xC0 | (codepoint >> 6));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0xFFFF) {
        result += static_cast<char>(0xE0 | (codepoint >> 12));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    } else if (codepoint <= 0x10FFFF) {
        result += static_cast<char>(0xF0 | (codepoint >> 18));
        result += static_cast<char>(0x80 | ((codepoint >> 12) & 0x3F));
        result += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
        result += static_cast<char>(0x80 | (codepoint & 0x3F));
    }
    return result;
}

void RawInput::OnKeyEvent(int glfwKey, int action, int mods) {
    (void)mods;
    if (glfwKey < 0 || glfwKey >= KEY_COUNT) return;

    if (glfwKey == GLFW_KEY_KP_ENTER) {
        glfwKey = GLFW_KEY_ENTER;
    }

    switch (action) {
        case GLFW_PRESS:
        case GLFW_REPEAT:
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

void RawInput::OnCharEvent(unsigned int codepoint) {
    s_charBuffer += CodepointToUTF8(codepoint);
}

std::string RawInput::ConsumeCharBuffer() {
    std::string result = std::move(s_charBuffer);
    s_charBuffer.clear();
    return result;
}

void RawInput::OnMouseButtonEvent(int glfwButton, int action) {
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

void RawInput::OnMouseMoveEvent(double x, double y) {
    s_state.mouseX = x;
    s_state.mouseY = y;
}

void RawInput::OnScrollEvent(double xOffset, double yOffset) {
    s_state.scrollX += xOffset;
    s_state.scrollY += yOffset;
}

void RawInput::SetClipboardText(const std::string& text) {
    if (!OpenClipboard(nullptr)) return;
    EmptyClipboard();

    HGLOBAL hMem = GlobalAlloc(GMEM_MOVEABLE, text.size() + 1);
    if (hMem) {
        char* dst = static_cast<char*>(GlobalLock(hMem));
        if (dst) {
            memcpy(dst, text.c_str(), text.size() + 1);
            GlobalUnlock(hMem);
            SetClipboardData(CF_TEXT, hMem);
        }
    }

    CloseClipboard();
}

std::string RawInput::GetClipboardText() {
    if (!OpenClipboard(nullptr)) return {};

    HANDLE hData = GetClipboardData(CF_TEXT);
    std::string result;
    if (hData) {
        const char* src = static_cast<const char*>(GlobalLock(hData));
        if (src) {
            result = src;
            GlobalUnlock(hData);
        }
    }

    CloseClipboard();
    return result;
}

void RawInput::Update() {
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

    for (int i = 0; i < GAMEPAD_COUNT; ++i) {
        s_gamepadConnected[i] = (glfwJoystickPresent(GLFW_JOYSTICK_1 + i) &&
                                 glfwJoystickIsGamepad(GLFW_JOYSTICK_1 + i));
    }

    s_state.scrollX = 0.0;
    s_state.scrollY = 0.0;
}
