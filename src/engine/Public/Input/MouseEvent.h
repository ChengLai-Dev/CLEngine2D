#pragma once

#include <Math/Vec2.h>
#include <string>

struct MouseEvent {
    enum ActionType { Down, Move, Up, Scroll };
    enum ButtonType { None, Left, Right, Middle };
    ActionType type;
    Vec2 screenPos;
    ButtonType button = None;
    float scrollDelta = 0.0f;

    std::string GetTypeString() const {
        switch (type) {
            case Down:   return "Down";
            case Move:   return "Move";
            case Up:     return "Up";
            case Scroll: return "Scroll";
            default:     return "Unknown";
        }
    }

    std::string GetButtonString() const {
        switch (button) {
            case None:   return "None";
            case Left:   return "Left";
            case Right:  return "Right";
            case Middle: return "Middle";
            default:     return "Unknown";
        }
    }
};
