#pragma once

#include <memory>
#include <string>

struct ma_engine;

class Sound;

class AudioEngine {
public:
    static AudioEngine& GetInstance();

    bool Init();
    void Shutdown();

    std::shared_ptr<Sound> LoadSound(const std::string& filepath);

    void SetListenerPosition(float x, float y);
    void SetListenerPosition(const struct Vec3& position);

    ma_engine* GetNativeEngine() const;

private:
    AudioEngine();
    ~AudioEngine();

    AudioEngine(const AudioEngine&) = delete;
    AudioEngine& operator=(const AudioEngine&) = delete;

    ma_engine* m_engine = nullptr;
};
