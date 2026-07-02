#include "Input/InputAction.h"

void InputAction::OnStarted(Callback cb) {
    m_onStarted.push_back(std::move(cb));
}

void InputAction::OnTriggered(Callback cb) {
    m_onTriggered.push_back(std::move(cb));
}

void InputAction::OnCompleted(Callback cb) {
    m_onCompleted.push_back(std::move(cb));
}

void InputAction::Update() {
    bool wasActive = !m_prevValue.IsZero();
    bool isActive = !m_value.IsZero();

    if (!wasActive && isActive) {
        for (std::size_t i = 0; i < m_onStarted.size(); ++i) {
            m_onStarted[i](m_value);
        }
    }

    if (isActive) {
        for (std::size_t i = 0; i < m_onTriggered.size(); ++i) {
            m_onTriggered[i](m_value);
        }
    }

    if (wasActive && !isActive) {
        for (std::size_t i = 0; i < m_onCompleted.size(); ++i) {
            m_onCompleted[i](m_prevValue);
        }
    }
}
