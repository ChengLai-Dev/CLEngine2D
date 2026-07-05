#include "Input/InputSystem.h"
#include "Input/InputAction.h"
#include "Input/InputMappingContext.h"
#include "Input/RawInput.h"
#include <algorithm>
#include <cmath>
#include <cstddef>
#include <unordered_set>
#include <vector>

InputSystem& InputSystem::GetInstance() {
    static InputSystem instance;
    return instance;
}

void InputSystem::AddContext(std::shared_ptr<InputMappingContext> context, int priority) {
    m_contexts.push_back({std::move(context), priority});
    std::sort(m_contexts.begin(), m_contexts.end(),
        [](const ContextEntry& a, const ContextEntry& b) {
            return a.priority > b.priority;
        });
    m_mappingsDirty = true;
}

void InputSystem::RemoveContext(InputMappingContext* context) {
    for (std::size_t i = 0; i < m_contexts.size(); ++i) {
        if (m_contexts[i].context.get() == context) {
            m_contexts.erase(m_contexts.begin() + static_cast<std::ptrdiff_t>(i));
            m_mappingsDirty = true;
            return;
        }
    }
}

void InputSystem::SetInputMode(EInputMode mode) {
    if (m_inputMode != mode) {
        m_inputMode = mode;
        m_mappingsDirty = true;
    }
}

void InputSystem::ResetUIConsumedFlags() {
    m_uiConsumedKeys.reset();
    m_uiConsumedMouse.reset();
}

void InputSystem::MarkKeyConsumedByUI(uint16_t code, bool isMouse) {
    if (isMouse) {
        if (code < kMaxMouse) {
            m_uiConsumedMouse.set(code);
        }
    } else {
        if (code < kMaxKeys) {
            m_uiConsumedKeys.set(code);
        }
    }
}

void InputSystem::RebuildControlMappings() {
    // Snapshot actions that were previously mapped (for mid-hold consumption handling)
    std::unordered_set<InputAction*> oldActiveActions;
    for (const auto& cm : m_compiledMappings) {
        if (cm.action->IsActive()) {
            oldActiveActions.insert(cm.action.get());
        }
    }

    std::bitset<kMaxKeys> consumedKeys;
    std::bitset<kMaxMouse> consumedMouse;

    m_compiledMappings.clear();

    // Pre-mark UI-consumed keys in GameAndUI mode
    if (m_inputMode == EInputMode::GameAndUI) {
        consumedKeys |= m_uiConsumedKeys;
        consumedMouse |= m_uiConsumedMouse;
    }

    for (const ContextEntry& entry : m_contexts) {
        InputMappingContext* ctx = entry.context.get();

        // Input mode filtering — skip IMCs that don't match current mode
        if (m_inputMode == EInputMode::GameOnly && ctx->InputMode != EInputMode::GameOnly) {
            continue;
        }
        if (m_inputMode == EInputMode::UIOnly && ctx->InputMode != EInputMode::UIOnly) {
            continue;
        }
        // GameAndUI: both GameOnly and UIOnly IMCs are processed

        std::bitset<kMaxKeys> contextConsumedKeys;
        std::bitset<kMaxMouse> contextConsumedMouse;

        for (const ActionMapping& mapping : ctx->GetMappings()) {
            if (!mapping.action) continue;

            bool consumed = false;
            switch (mapping.device) {
                case EDevice::Keyboard:
                    if (mapping.code < kMaxKeys && consumedKeys.test(mapping.code)) {
                        consumed = true;
                    }
                    break;
                case EDevice::Mouse:
                    if (mapping.code < kMaxMouse && consumedMouse.test(mapping.code)) {
                        consumed = true;
                    }
                    break;
            }
            if (consumed) continue;

            m_compiledMappings.push_back({
                mapping.action,
                mapping.device,
                mapping.code,
                mapping.scale
            });

            // Mark key as consumed for lower-priority contexts
            if (mapping.action->bConsumeInput) {
                switch (mapping.device) {
                    case EDevice::Keyboard:
                        if (mapping.code < kMaxKeys) contextConsumedKeys.set(mapping.code);
                        break;
                    case EDevice::Mouse:
                        if (mapping.code < kMaxMouse) contextConsumedMouse.set(mapping.code);
                        break;
                }
            }
        }

        // Commit consumed keys after processing this context (UE5-aligned: per-context commit)
        consumedKeys |= contextConsumedKeys;
        consumedMouse |= contextConsumedMouse;
    }

    // Force-complete actions that were active but had their keys consumed mid-hold
    std::unordered_set<InputAction*> newActions;
    for (const auto& cm : m_compiledMappings) {
        newActions.insert(cm.action.get());
    }
    for (InputAction* action : oldActiveActions) {
        if (newActions.find(action) == newActions.end()) {
            action->ForceComplete();
        }
    }

    m_mappingsDirty = false;
}

void InputSystem::Update() {
    RawInput::Update();

    if (m_mappingsDirty) {
        RebuildControlMappings();
    }

    // Collect unique actions from compiled mappings
    std::vector<InputAction*> actions;
    for (const auto& cm : m_compiledMappings) {
        bool found = false;
        for (InputAction* a : actions) {
            if (a == cm.action.get()) { found = true; break; }
        }
        if (!found) {
            actions.push_back(cm.action.get());
        }
    }

    // Reset action values
    for (InputAction* action : actions) {
        action->m_prevValue = action->m_value;
        action->m_value = InputActionValue();
    }

    // Accumulate values from compiled mappings (with priority already resolved)
    for (const auto& cm : m_compiledMappings) {
        InputAction* action = cm.action.get();

        // In GameAndUI mode, skip keys consumed by UI this frame
        if (m_inputMode == EInputMode::GameAndUI) {
            bool consumedByUI = false;
            switch (cm.device) {
                case EDevice::Keyboard:
                    if (cm.code < kMaxKeys) consumedByUI = m_uiConsumedKeys.test(cm.code);
                    break;
                case EDevice::Mouse:
                    if (cm.code < kMaxMouse) consumedByUI = m_uiConsumedMouse.test(cm.code);
                    break;
            }
            if (consumedByUI) continue;
        }

        bool down = false;
        switch (cm.device) {
            case EDevice::Keyboard:
                down = RawInput::IsKeyDown(static_cast<KeyCode>(cm.code));
                break;
            case EDevice::Mouse:
                down = RawInput::IsMouseButtonDown(static_cast<MouseCode>(cm.code));
                break;
        }

        if (!down) continue;

        // UE5-aligned accumulation behavior
        if (action->AccumulationBehavior == EInputActionAccumulationBehavior::Cumulative) {
            action->m_value.x += cm.scale.x;
            action->m_value.y += cm.scale.y;
        } else {
            // TakeHighestAbsoluteValue — per-component
            if (std::abs(cm.scale.x) >= std::abs(action->m_value.x)) {
                action->m_value.x = cm.scale.x;
            }
            if (std::abs(cm.scale.y) >= std::abs(action->m_value.y)) {
                action->m_value.y = cm.scale.y;
            }
        }
    }

    // Fire callbacks
    for (InputAction* action : actions) {
        action->Update();
    }
}

void InputSystem::Clear() {
    m_contexts.clear();
    m_compiledMappings.clear();
    m_uiConsumedKeys.reset();
    m_uiConsumedMouse.reset();
    m_mappingsDirty = true;
}
