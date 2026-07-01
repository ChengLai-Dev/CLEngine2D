#include "Timer.h"
#include "Platform/Window.h"

Timer::Timer() {
    m_startTime = Window::GetTime();
    m_lastFrameTime = m_startTime;
}

void Timer::TickFrame() {
    double currentTime = Window::GetTime();
    m_deltaTime = static_cast<float>(currentTime - m_lastFrameTime);
    m_lastFrameTime = currentTime;

    m_totalFrameTime -= m_frameTimes[m_frameIndex];
    m_frameTimes[m_frameIndex] = m_deltaTime;
    m_totalFrameTime += m_deltaTime;
    m_frameIndex = (m_frameIndex + 1) % SAMPLE_COUNT;

    if (m_sampleCount < SAMPLE_COUNT) {
        m_sampleCount++;
    }

    m_frameTime = m_totalFrameTime / static_cast<float>(m_sampleCount);
    m_fps = (m_frameTime > 0.0f) ? 1.0f / m_frameTime : 0.0f;
}

float Timer::GetElapsed() const {
    return static_cast<float>(Window::GetTime() - m_startTime);
}

void Timer::Reset() {
    m_startTime = Window::GetTime();
    m_lastFrameTime = m_startTime;
    m_deltaTime = 0.0f;
    m_fps = 0.0f;
    m_frameTime = 0.0f;
    for (int i = 0; i < SAMPLE_COUNT; i++) {
        m_frameTimes[i] = 0.0f;
    }
    m_frameIndex = 0;
    m_sampleCount = 0;
    m_totalFrameTime = 0.0f;
}
