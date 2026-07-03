#pragma once

#include "Math/Vec2.h"
#include "Math/Vec3.h"
#include "Math/Vec4.h"

#include <memory>
#include <vector>

class Texture;
class Renderer;

struct Particle {
    Vec3 position;
    Vec3 velocity;
    Vec4 color;
    Vec4 startColor;
    Vec4 endColor;
    float life = 0.0f;
    float maxLife = 1.0f;
    float size = 1.0f;
    float startSize = 1.0f;
    float endSize = 0.0f;
    float rotation = 0.0f;
    bool active = false;
};

enum class EmissionShape {
    Point,
    Circle,
    CircleEdge
};

struct ParticleEmitterParams {
    float emissionRate = 50.0f;
    int maxParticles = 500;

    EmissionShape shape = EmissionShape::Circle;
    float radius = 1.0f;

    float lifetimeMin = 0.5f;
    float lifetimeMax = 2.0f;

    float speedMin = 1.0f;
    float speedMax = 3.0f;

    Vec3 direction = Vec3(0.0f, 1.0f, 0.0f);
    float spreadAngle = 0.0f;

    Vec4 startColor = Vec4(1.0f, 1.0f, 1.0f, 1.0f);
    Vec4 endColor = Vec4(0.0f, 0.0f, 0.0f, 0.0f);

    float startSizeMin = 0.5f;
    float startSizeMax = 1.0f;
    float endSizeMin = 0.0f;
    float endSizeMax = 0.0f;

    Vec3 gravity = Vec3(0.0f, 0.0f, 0.0f);

    bool oneShot = false;
};

class ParticleEmitter {
public:
    ParticleEmitter();
    ~ParticleEmitter();

    void SetParams(const ParticleEmitterParams& params);
    const ParticleEmitterParams& GetParams() const;
    ParticleEmitterParams& GetParams();

    void SetTexture(std::shared_ptr<Texture> texture);
    std::shared_ptr<Texture> GetTexture() const;

    void SetPosition(const Vec3& pos);
    const Vec3& GetPosition() const;

    void Play();
    void Stop();
    void Burst(int count);
    bool IsPlaying() const;

    void Update(float deltaTime);
    void Render(Renderer& renderer) const;

    int GetActiveCount() const;

private:
    void SpawnParticle();
    void ResetParticle(Particle& p);

    ParticleEmitterParams m_params;
    std::vector<Particle> m_particles;
    std::shared_ptr<Texture> m_texture = nullptr;
    Vec3 m_position;
    bool m_playing = false;
    float m_spawnAccumulator = 0.0f;
    float m_defaultColor[4] = { 1.0f, 1.0f, 1.0f, 1.0f };
};
