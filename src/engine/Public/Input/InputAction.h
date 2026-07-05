#pragma once

#include "Math/Vec2.h"
#include <cstdint>
#include <functional>
#include <vector>

enum class EInputActionAccumulationBehavior : uint8_t {
    TakeHighestAbsoluteValue,
    Cumulative
};

struct InputActionValue {
    float x = 0.0f;
    float y = 0.0f;

    bool IsZero() const { return x == 0.0f && y == 0.0f; }
    Vec2 GetVec2() const { return Vec2(x, y); }
    float GetAxis() const { return x; }
};

class InputAction {
public:
    using Callback = std::function<void(const InputActionValue&)>;

    InputAction() = default;
    ~InputAction() = default;

    InputAction(const InputAction&) = delete;
    InputAction& operator=(const InputAction&) = delete;

    void OnStarted(Callback cb);
    void OnTriggered(Callback cb);
    void OnCompleted(Callback cb);

    const InputActionValue& GetValue() const { return m_value; }
    bool IsActive() const { return !m_value.IsZero(); }

    bool bConsumeInput = true;
    EInputActionAccumulationBehavior AccumulationBehavior =
        EInputActionAccumulationBehavior::TakeHighestAbsoluteValue;

private:
    friend class InputSystem;

    void Update();
    void ForceComplete();

    InputActionValue m_value;
    InputActionValue m_prevValue;

    std::vector<Callback> m_onStarted;
    std::vector<Callback> m_onTriggered;
    std::vector<Callback> m_onCompleted;
};
