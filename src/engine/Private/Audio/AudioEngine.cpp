#include "Audio/Sound.h"
#include "Audio/AudioEngine.h"
#include "Logger.h"
#include "Math/Vec3.h"

#define MINIAUDIO_IMPLEMENTATION
#include "miniaudio.h"
#undef MINIAUDIO_IMPLEMENTATION


AudioEngine::AudioEngine() = default;

AudioEngine::~AudioEngine() {
    Shutdown();
}

AudioEngine& AudioEngine::GetInstance() {
    static AudioEngine instance;
    return instance;
}

bool AudioEngine::Init() {
    if (m_engine) {
        Logger::Warn("Audio engine already initialized");
        return true;
    }

    m_engine = new ma_engine();
    ma_engine_config config = ma_engine_config_init();

    if (ma_engine_init(&config, m_engine) != MA_SUCCESS) {
        Logger::Error("Failed to initialize miniaudio engine");
        delete m_engine;
        m_engine = nullptr;
        return false;
    }

    Logger::Info("Audio engine initialized");
    return true;
}

void AudioEngine::Shutdown() {
    if (m_engine) {
        ma_engine_uninit(m_engine);
        delete m_engine;
        m_engine = nullptr;
        Logger::Info("Audio engine shutdown");
    }
}

std::shared_ptr<Sound> AudioEngine::LoadSound(const std::string& filepath) {
    if (!m_engine) {
        Logger::Error("Audio engine not initialized, call Init() first");
        return nullptr;
    }

    std::shared_ptr<Sound> sound(new Sound(m_engine));
    if (!sound->Load(filepath)) {
        Logger::Error("Failed to load sound: {}", filepath);
        return nullptr;
    }
    return sound;
}

void AudioEngine::SetListenerPosition(float x, float y) {
    if (m_engine) {
        ma_engine_listener_set_position(m_engine, 0, x, y, 0.0f);
    }
}

void AudioEngine::SetListenerPosition(const Vec3& position) {
    SetListenerPosition(position.x, position.y);
}

ma_engine* AudioEngine::GetNativeEngine() const {
    return m_engine;
}
