#include "miniaudio.h"

#include "Audio/Sound.h"
#include "Logger.h"

#include <format>

Sound::Sound(ma_engine* engine)
    : m_engine(engine)
{
}

Sound::~Sound() {
    Stop();
    if (m_sound) {
        ma_sound_uninit(m_sound);
        delete m_sound;
        m_sound = nullptr;
    }
}

bool Sound::Load(const std::string& filepath) {
    if (m_sound) {
        ma_sound_uninit(m_sound);
        delete m_sound;
        m_sound = nullptr;
    }

    m_sound = new ma_sound();
    ma_result result = ma_sound_init_from_file(m_engine, filepath.c_str(),
        MA_SOUND_FLAG_NO_DEFAULT_ATTACHMENT, nullptr, nullptr, m_sound);

    if (result != MA_SUCCESS) {
        Logger::Error(std::format("Failed to load sound: {}", filepath));
        delete m_sound;
        m_sound = nullptr;
        return false;
    }

    return true;
}

void Sound::Play() {
    if (m_sound) {
        ma_sound_start(m_sound);
    }
}

void Sound::Stop() {
    if (m_sound) {
        ma_sound_stop(m_sound);
        ma_sound_seek_to_pcm_frame(m_sound, 0);
    }
}

void Sound::Pause() {
    if (m_sound) {
        ma_sound_stop(m_sound);
    }
}

void Sound::Resume() {
    Play();
}

void Sound::SetVolume(float volume) {
    m_volume = volume;
    if (m_sound) {
        ma_sound_set_volume(m_sound, volume);
    }
}

float Sound::GetVolume() const {
    return m_volume;
}

void Sound::SetLooping(bool loop) {
    if (m_sound) {
        ma_sound_set_looping(m_sound, loop ? MA_TRUE : MA_FALSE);
    }
}

bool Sound::IsLooping() const {
    if (m_sound) {
        return ma_sound_is_looping(m_sound) != MA_FALSE;
    }
    return false;
}

void Sound::SetPosition(float x, float y) {
    if (m_sound) {
        ma_sound_set_position(m_sound, x, y, 0.0f);
    }
}

void Sound::SetMaxDistance(float distance) {
    m_maxDistance = distance;
    if (m_sound) {
        ma_sound_set_max_distance(m_sound, distance);
    }
}

void Sound::SetMinDistance(float distance) {
    m_minDistance = distance;
    if (m_sound) {
        ma_sound_set_min_distance(m_sound, distance);
    }
}

void Sound::SetRolloff(float rolloff) {
    m_rolloff = rolloff;
    if (m_sound) {
        ma_sound_set_rolloff(m_sound, rolloff);
    }
}

bool Sound::IsPlaying() const {
    if (m_sound) {
        return ma_sound_is_playing(m_sound) != MA_FALSE;
    }
    return false;
}

bool Sound::IsPaused() const {
    if (m_sound) {
        return !ma_sound_is_playing(m_sound) &&
               ma_sound_get_time_in_pcm_frames(m_sound) > 0;
    }
    return false;
}

bool Sound::IsStopped() const {
    if (m_sound) {
        return !ma_sound_is_playing(m_sound) &&
               ma_sound_get_time_in_pcm_frames(m_sound) == 0;
    }
    return true;
}

float Sound::GetDuration() const {
    if (m_sound && m_engine) {
        ma_uint64 length;
        if (ma_sound_get_length_in_pcm_frames(m_sound, &length) == MA_SUCCESS) {
            float sampleRate = static_cast<float>(ma_engine_get_sample_rate(m_engine));
            if (sampleRate > 0.0f) {
                return static_cast<float>(length) / sampleRate;
            }
        }
    }
    return 0.0f;
}
