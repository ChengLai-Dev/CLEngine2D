#pragma once

#include <Math/Vec2.h>
#include <string>

struct MouseEvent {
    enum ActionType { Press, Move, Release, Scroll };
    enum ButtonType { None, Left, Right, Middle };
    ActionType type;
    Vec2 screenPos;
    ButtonType button = None;
    float scrollDelta = 0.0f;

    std::string GetTypeString() const {
        switch (type) {
            case Press:   return "Press";
            case Move:    return "Move";
            case Release: return "Release";
            case Scroll:  return "Scroll";
            default:      return "Unknown";
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
