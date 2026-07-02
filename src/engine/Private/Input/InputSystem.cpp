#include "Input/InputSystem.h"
#include "Input/InputAction.h"
#include "Input/InputMappingContext.h"
#include "Input/RawInput.h"
#include <algorithm>
#include <cstddef>
#include <vector>

InputSystem& InputSystem::GetInstance() {
    static InputSystem instance;
    return instance;
}

void InputSystem::AddContext(InputMappingContext* context, int priority) {
    m_contexts.push_back({context, priority});
    std::sort(m_contexts.begin(), m_contexts.end(),
        [](const ContextEntry& a, const ContextEntry& b) {
            return a.priority > b.priority;
        });
}

void InputSystem::RemoveContext(InputMappingContext* context) {
    for (std::size_t i = 0; i < m_contexts.size(); ++i) {
        if (m_contexts[i].context == context) {
            m_contexts.erase(m_contexts.begin() + static_cast<std::ptrdiff_t>(i));
            return;
        }
    }
}

void InputSystem::Update() {
    RawInput::Update();

    std::vector<InputAction*> actions;
    for (ContextEntry& entry : m_contexts) {
        for (const InputMappingContext::Mapping& mapping : entry.context->GetMappings()) {
            if (!mapping.action) continue;

            bool found = false;
            for (InputAction* a : actions) {
                if (a == mapping.action) { found = true; break; }
            }
            if (!found) {
                actions.push_back(mapping.action);
            }
        }
    }

    for (InputAction* action : actions) {
        action->m_prevValue = action->m_value;
        action->m_value = InputActionValue();
    }

    for (ContextEntry& entry : m_contexts) {
        for (const InputMappingContext::Mapping& mapping : entry.context->GetMappings()) {
            if (!mapping.action) continue;

            bool down = false;
            switch (mapping.device) {
                case InputMappingContext::Mapping::Device::Keyboard:
                    down = RawInput::IsKeyDown(static_cast<KeyCode>(mapping.code));
                    break;
                case InputMappingContext::Mapping::Device::Mouse:
                    down = RawInput::IsMouseButtonDown(static_cast<MouseCode>(mapping.code));
                    break;
            }

            if (down) {
                mapping.action->m_value.x += mapping.scale.x;
                mapping.action->m_value.y += mapping.scale.y;
            }
        }
    }

    for (InputAction* action : actions) {
        action->Update();
    }
}

void InputSystem::Clear() {
    m_contexts.clear();
}
