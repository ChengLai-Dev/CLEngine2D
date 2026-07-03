#include "Animator.h"
#include "SceneGraph/Sprite.h"

#include <algorithm>

Animator::Animator() = default;
Animator::~Animator() = default;

void Animator::SetTarget(Sprite* sprite) {
    m_target = sprite;
}

Sprite* Animator::GetTarget() const {
    return m_target;
}

void Animator::AddClip(const AnimationClip& clip) {
    for (auto& existing : m_clips) {
        if (existing.name == clip.name) {
            existing = clip;
            return;
        }
    }
    m_clips.push_back(clip);
}

bool Animator::HasClip(const std::string& name) const {
    for (const auto& clip : m_clips) {
        if (clip.name == name) return true;
    }
    return false;
}

void Animator::Play(const std::string& clipName) {
    for (auto& clip : m_clips) {
        if (clip.name == clipName) {
            m_currentClip = &clip;
            m_currentFrame = 0;
            m_elapsed = 0.0f;
            m_playing = true;
            m_paused = false;

            if (!m_currentClip->frames.empty()) {
                ApplyFrame(m_currentClip->frames[0]);
            }
            return;
        }
    }
}

void Animator::Stop() {
    m_playing = false;
    m_paused = false;
    m_currentClip = nullptr;
    m_currentFrame = 0;
    m_elapsed = 0.0f;
}

void Animator::Pause() {
    m_paused = true;
}

void Animator::Resume() {
    if (m_playing) {
        m_paused = false;
    }
}

bool Animator::IsPlaying() const {
    return m_playing && !m_paused;
}

bool Animator::IsPaused() const {
    return m_paused;
}

const std::string& Animator::GetCurrentClipName() const {
    static const std::string empty;
    if (m_currentClip) return m_currentClip->name;
    return empty;
}

int Animator::GetCurrentFrameIndex() const {
    return m_currentFrame;
}

void Animator::SetSpeed(float speed) {
    m_speed = std::max(0.0f, speed);
}

float Animator::GetSpeed() const {
    return m_speed;
}

void Animator::SetOnFinished(std::function<void()> callback) {
    m_onFinished = std::move(callback);
}

void Animator::Update(float deltaTime) {
    if (!m_playing || m_paused || !m_currentClip) return;
    if (m_currentClip->frames.empty()) return;

    m_elapsed += deltaTime * m_speed;

    const auto& frames = m_currentClip->frames;
    const AnimationFrame& currentFrameData = frames[m_currentFrame];

    if (m_elapsed >= currentFrameData.duration) {
        m_elapsed -= currentFrameData.duration;
        m_currentFrame++;

        if (m_currentFrame >= static_cast<int>(frames.size())) {
            if (m_currentClip->looping) {
                m_currentFrame = 0;
            } else {
                m_currentFrame = static_cast<int>(frames.size()) - 1;
                m_playing = false;
                if (m_onFinished) {
                    m_onFinished();
                }
                return;
            }
        }

        ApplyFrame(frames[m_currentFrame]);
    }
}

void Animator::ApplyFrame(const AnimationFrame& frame) {
    if (!m_target) return;

    m_target->SetTexOffset(frame.texOffsetX, frame.texOffsetY);
    m_target->SetTexScale(frame.texScaleX, frame.texScaleY);
}
