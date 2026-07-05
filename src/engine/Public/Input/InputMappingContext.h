#pragma once

#include "Input/InputCodes.h"
#include "Math/Vec2.h"
#include <cstdint>
#include <memory>
#include <vector>

class InputAction;

enum class EInputMode : uint8_t {
    GameOnly,
    UIOnly,
    GameAndUI
};

enum class EDevice : uint8_t { Keyboard, Mouse };

struct ActionMapping {
    std::shared_ptr<InputAction> action;
    EDevice device;
    uint16_t code;
    Vec2 scale;
};

class InputMappingContext {
public:
    void MapKey(std::shared_ptr<InputAction> action, KeyCode key, const Vec2& scale = Vec2(1.0f, 0.0f));
    void MapMouse(std::shared_ptr<InputAction> action, MouseCode button, const Vec2& scale = Vec2(1.0f, 0.0f));

    const std::vector<ActionMapping>& GetMappings() const { return m_mappings; }

    EInputMode InputMode = EInputMode::GameOnly;

private:
    std::vector<ActionMapping> m_mappings;
};
