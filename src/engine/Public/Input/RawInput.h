#pragma once

#include "Input/InputCodes.h"
#include "Math/Vec2.h"
#include <string>

class RawInput {
public:
    enum class KeyState { None, Pressed, Held, Released };

    static bool IsKeyDown(KeyCode key);
    static bool IsKeyPressed(KeyCode key);
    static bool IsKeyReleased(KeyCode key);

    static bool IsMouseButtonDown(MouseCode button);
    static bool IsMouseButtonPressed(MouseCode button);
    static bool IsMouseButtonReleased(MouseCode button);
    static float GetMouseX();
    static float GetMouseY();
    static Vec2 GetMousePosition();
    static float GetScrollDeltaX();
    static float GetScrollDeltaY();

    static bool IsGamepadConnected(int gamepadIndex = 0);
    static float GetGamepadAxis(GamepadAxis axis, int gamepadIndex = 0);
    static bool IsGamepadButtonDown(GamepadButton button, int gamepadIndex = 0);
    static bool IsGamepadButtonPressed(GamepadButton button, int gamepadIndex = 0);

    static void OnKeyEvent(int glfwKey, int action, int mods);
    static void OnCharEvent(unsigned int codepoint);
    static void OnMouseButtonEvent(int glfwButton, int action);
    static void OnMouseMoveEvent(double x, double y);
    static void OnScrollEvent(double xOffset, double yOffset);

    static std::string ConsumeCharBuffer();

    static void Update();

private:
    static constexpr int KEY_COUNT = 349;
    static constexpr int MOUSE_COUNT = 8;
    static constexpr int GAMEPAD_COUNT = 4;
    static constexpr int GAMEPAD_BUTTON_COUNT = 15;
    static constexpr int GAMEPAD_AXIS_COUNT = 6;

    struct InputState {
        KeyState keyStates[KEY_COUNT]{};
        KeyState mouseStates[MOUSE_COUNT]{};
        double mouseX = 0.0;
        double mouseY = 0.0;
        double scrollX = 0.0;
        double scrollY = 0.0;
    };

    static InputState s_state;
    static bool s_gamepadConnected[GAMEPAD_COUNT];
    static std::string s_charBuffer;
};
