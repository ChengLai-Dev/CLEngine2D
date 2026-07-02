#pragma once

#include "Input/InputCodes.h"
#include "Math/Vec2.h"
#include <cstdint>
#include <vector>

class InputAction;

class InputMappingContext {
public:
    struct Mapping {
        InputAction* action;
        enum class Device : uint8_t { Keyboard, Mouse };
        Device device;
        uint16_t code;
        Vec2 scale;
    };

    void MapKey(InputAction* action, KeyCode key, const Vec2& scale = Vec2(1.0f, 0.0f));
    void MapMouse(InputAction* action, MouseCode button, const Vec2& scale = Vec2(1.0f, 0.0f));

    const std::vector<Mapping>& GetMappings() const { return m_mappings; }

private:
    std::vector<Mapping> m_mappings;
};
