#pragma once

#include <memory>
#include <string>

struct ma_sound;
struct ma_engine;

class Sound {
public:
    ~Sound();

    bool Load(const std::string& filepath);

    void Play();
    void Stop();
    void Pause();
    void Resume();

    void SetVolume(float volume);
    float GetVolume() const;

    void SetLooping(bool loop);
    bool IsLooping() const;

    void SetPosition(float x, float y);
    void SetMaxDistance(float distance);
    void SetMinDistance(float distance);
    void SetRolloff(float rolloff);

    bool IsPlaying() const;
    bool IsPaused() const;
    bool IsStopped() const;

    float GetDuration() const;

private:
    friend class AudioEngine;
    Sound(ma_engine* engine);

    ma_engine* m_engine;
    ma_sound* m_sound = nullptr;
    float m_volume = 1.0f;
    float m_maxDistance = 50.0f;
    float m_minDistance = 5.0f;
    float m_rolloff = 1.0f;
};
