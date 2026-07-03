#pragma once

#include <functional>
#include <memory>
#include <string>
#include <vector>

class Sprite;

struct AnimationFrame {
    float texOffsetX = 0.0f;
    float texOffsetY = 0.0f;
    float texScaleX = 1.0f;
    float texScaleY = 1.0f;
    float duration = 0.1f;
};

struct AnimationClip {
    std::string name;
    std::vector<AnimationFrame> frames;
    bool looping = true;
};

class Animator {
public:
    Animator();
    ~Animator();

    void SetTarget(Sprite* sprite);
    Sprite* GetTarget() const;

    void AddClip(const AnimationClip& clip);
    bool HasClip(const std::string& name) const;

    void Play(const std::string& clipName);
    void Stop();
    void Pause();
    void Resume();

    bool IsPlaying() const;
    bool IsPaused() const;

    const std::string& GetCurrentClipName() const;
    int GetCurrentFrameIndex() const;

    void SetSpeed(float speed);
    float GetSpeed() const;

    void SetOnFinished(std::function<void()> callback);

    void Update(float deltaTime);

private:
    void ApplyFrame(const AnimationFrame& frame);

    Sprite* m_target = nullptr;
    std::vector<AnimationClip> m_clips;
    AnimationClip* m_currentClip = nullptr;

    int m_currentFrame = 0;
    float m_elapsed = 0.0f;
    float m_speed = 1.0f;
    bool m_playing = false;
    bool m_paused = false;

    std::function<void()> m_onFinished = nullptr;
};
