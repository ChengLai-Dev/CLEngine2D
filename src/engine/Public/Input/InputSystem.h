#pragma once

#include "Input/InputMappingContext.h"
#include "Math/Vec2.h"
#include <bitset>
#include <memory>
#include <vector>

class InputAction;

class InputSystem {
public:
    static InputSystem& GetInstance();

    void AddContext(std::shared_ptr<InputMappingContext> context, int priority = 0);
    void RemoveContext(InputMappingContext* context);

    void SetInputMode(EInputMode mode);
    EInputMode GetInputMode() const { return m_inputMode; }

    void MarkKeyConsumedByUI(uint16_t code, bool isMouse = false);
    void ResetUIConsumedFlags();

    void Update();
    void EndFrame();
    void Clear();

    ~InputSystem() = default;

private:
    InputSystem() = default;
    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;

    struct ContextEntry {
        std::shared_ptr<InputMappingContext> context;
        int priority;
    };

    void RebuildControlMappings();

    static constexpr std::size_t kMaxKeys = 349;
    static constexpr std::size_t kMaxMouse = 8;

    std::vector<ContextEntry> m_contexts;
    std::vector<ActionMapping> m_compiledMappings;

    EInputMode m_inputMode = EInputMode::GameOnly;

    std::bitset<kMaxKeys> m_uiConsumedKeys;
    std::bitset<kMaxMouse> m_uiConsumedMouse;

    bool m_mappingsDirty = true;
};
