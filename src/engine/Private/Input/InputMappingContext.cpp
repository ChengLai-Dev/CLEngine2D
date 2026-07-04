#include "Input/InputMappingContext.h"
#include "Input/InputAction.h"

void InputMappingContext::MapKey(std::shared_ptr<InputAction> action, KeyCode key, const Vec2& scale) {
    m_mappings.push_back({std::move(action), Mapping::Device::Keyboard, static_cast<uint16_t>(key), scale});
}

void InputMappingContext::MapMouse(std::shared_ptr<InputAction> action, MouseCode button, const Vec2& scale) {
    m_mappings.push_back({std::move(action), Mapping::Device::Mouse, static_cast<uint16_t>(button), scale});
}
