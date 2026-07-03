#pragma once

#include <memory>
#include <vector>

class InputAction;
class InputMappingContext;

class InputSystem {
public:
    static InputSystem& GetInstance();

    void AddContext(std::shared_ptr<InputMappingContext> context, int priority = 0);
    void RemoveContext(InputMappingContext* context);
    void Update();
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

    std::vector<ContextEntry> m_contexts;
};
