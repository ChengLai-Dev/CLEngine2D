#pragma once

#include <vector>

class InputAction;
class InputMappingContext;

class InputSystem {
public:
    static InputSystem& GetInstance();

    void AddContext(InputMappingContext* context, int priority = 0);
    void RemoveContext(InputMappingContext* context);
    void Update();
    void Clear();

private:
    InputSystem() = default;
    ~InputSystem() = default;
    InputSystem(const InputSystem&) = delete;
    InputSystem& operator=(const InputSystem&) = delete;

    struct ContextEntry {
        InputMappingContext* context;
        int priority;
    };

    std::vector<ContextEntry> m_contexts;
};
