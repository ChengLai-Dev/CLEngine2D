#include "ParticleSystem.h"
#include "Render/Renderer.h"
#include "Render/Texture.h"

#include <cstdlib>
#include <algorithm>
#include <cmath>

static float RandFloat(float min, float max) {
    return min + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / (max - min)));
}

ParticleEmitter::ParticleEmitter() {
    m_particles.resize(m_params.maxParticles);
}

ParticleEmitter::~ParticleEmitter() = default;

void ParticleEmitter::SetParams(const ParticleEmitterParams& params) {
    m_params = params;
    if (static_cast<int>(m_particles.size()) != m_params.maxParticles) {
        m_particles.resize(m_params.maxParticles);
    }
}

const ParticleEmitterParams& ParticleEmitter::GetParams() const {
    return m_params;
}

ParticleEmitterParams& ParticleEmitter::GetParams() {
    return m_params;
}

void ParticleEmitter::SetTexture(std::shared_ptr<Texture> texture) {
    m_texture = std::move(texture);
}

std::shared_ptr<Texture> ParticleEmitter::GetTexture() const {
    return m_texture;
}

void ParticleEmitter::SetPosition(const Vec3& pos) {
    m_position = pos;
}

const Vec3& ParticleEmitter::GetPosition() const {
    return m_position;
}

void ParticleEmitter::Play() {
    m_playing = true;
    m_spawnAccumulator = 0.0f;
}

void ParticleEmitter::Stop() {
    m_playing = false;
}

void ParticleEmitter::Burst(int count) {
    for (int i = 0; i < count; ++i) {
        SpawnParticle();
    }
}

bool ParticleEmitter::IsPlaying() const {
    return m_playing;
}

void ParticleEmitter::Update(float deltaTime) {
    if (deltaTime > 0.05f) {
        deltaTime = 0.05f;
    }

    if (m_playing && m_params.emissionRate > 0.0f) {
        m_spawnAccumulator += deltaTime * m_params.emissionRate;
        while (m_spawnAccumulator >= 1.0f) {
            SpawnParticle();
            m_spawnAccumulator -= 1.0f;
        }
    }

    int activeCount = 0;
    for (Particle& p : m_particles) {
        if (!p.active) continue;

        p.life += deltaTime;
        if (p.life >= p.maxLife) {
            p.active = false;
            continue;
        }

        float t = p.life / p.maxLife;

        p.velocity += m_params.gravity * deltaTime;
        p.position += p.velocity * deltaTime;

        p.color.x = p.startColor.x + (p.endColor.x - p.startColor.x) * t;
        p.color.y = p.startColor.y + (p.endColor.y - p.startColor.y) * t;
        p.color.z = p.startColor.z + (p.endColor.z - p.startColor.z) * t;
        p.color.w = p.startColor.w + (p.endColor.w - p.startColor.w) * t;

        p.size = p.startSize + (p.endSize - p.startSize) * t;

        ++activeCount;
    }
}

void ParticleEmitter::Render(Renderer& renderer) const {
    Texture* tex = m_texture.get();

    for (const Particle& p : m_particles) {
        if (!p.active) continue;

        renderer.DrawQuad(p.position, Vec3(p.size, p.size, 1.0f),
                          p.rotation,
                          Color(p.color.x, p.color.y, p.color.z, p.color.w), tex);
    }
}

int ParticleEmitter::GetActiveCount() const {
    int count = 0;
    for (const Particle& p : m_particles) {
        if (p.active) ++count;
    }
    return count;
}

void ParticleEmitter::SpawnParticle() {
    for (Particle& p : m_particles) {
        if (!p.active) {
            ResetParticle(p);
            p.active = true;
            return;
        }
    }
}

void ParticleEmitter::ResetParticle(Particle& p) {
    p.maxLife = RandFloat(m_params.lifetimeMin, m_params.lifetimeMax);
    p.life = 0.0f;

    float speed = RandFloat(m_params.speedMin, m_params.speedMax);

    Vec3 emitDir = m_params.direction.Normalized();
    if (emitDir.LengthSq() < 0.001f) {
        emitDir = Vec3(0.0f, 1.0f, 0.0f);
    }

    if (m_params.spreadAngle > 0.0f) {
        float angleOffset = RandFloat(-m_params.spreadAngle, m_params.spreadAngle) * 0.5f;
        float cosA = std::cos(angleOffset);
        float sinA = std::sin(angleOffset);
        float dx = emitDir.x * cosA - emitDir.y * sinA;
        float dy = emitDir.x * sinA + emitDir.y * cosA;
        emitDir = Vec3(dx, dy, 0.0f).Normalized();
    }

    if (m_params.shape == EmissionShape::Circle) {
        float angle = RandFloat(0.0f, 6.2831853f);
        float r = RandFloat(0.0f, m_params.radius);
        p.position = m_position + Vec3(std::cos(angle) * r, std::sin(angle) * r, 0.0f);
    } else if (m_params.shape == EmissionShape::CircleEdge) {
        float angle = RandFloat(0.0f, 6.2831853f);
        p.position = m_position + Vec3(std::cos(angle) * m_params.radius, std::sin(angle) * m_params.radius, 0.0f);
    } else {
        p.position = m_position;
    }

    p.velocity = emitDir * speed;
    p.startColor = m_params.startColor;
    p.endColor = m_params.endColor;
    p.color = p.startColor;
    p.startSize = RandFloat(m_params.startSizeMin, m_params.startSizeMax);
    p.endSize = RandFloat(m_params.endSizeMin, m_params.endSizeMax);
    p.size = p.startSize;
    p.rotation = 0.0f;
}
