#pragma once

class Timer {
public:
    Timer();

    void TickFrame();

    float GetDeltaTime() const { return m_deltaTime; }
    float GetFPS() const { return m_fps; }
    float GetFrameTime() const { return m_frameTime; }
    float GetElapsed() const;

    void Reset();

private:
    static const int SAMPLE_COUNT = 60;

    double m_startTime;
    double m_lastFrameTime;
    float m_deltaTime = 0.0f;
    float m_fps = 0.0f;
    float m_frameTime = 0.0f;
    float m_frameTimes[SAMPLE_COUNT] = {};
    int m_frameIndex = 0;
    int m_sampleCount = 0;
    float m_totalFrameTime = 0.0f;
};
