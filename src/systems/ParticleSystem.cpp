#include "ParticleSystem.hpp"
#include "core/Constants.hpp"

#include <random>
#include <cmath>
#include <algorithm>

void ParticleSystem::emit(Vector2 pos, Vector2 vel, Color color, float life, float size, int count) {
    static std::mt19937 rng(std::random_device{}());
    std::uniform_real_distribution<float> dist(-1.0f, 1.0f);
    std::uniform_real_distribution<float> dist01(0.0f, 1.0f);

    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = pos;
        p.velocity = {
            vel.x + dist(rng) * 25.0f,
            vel.y + dist(rng) * 25.0f
        };
        p.color = color;
        p.life = life + dist01(rng) * life * 0.5f;
        p.maxLife = p.life;
        p.size = size + dist01(rng) * size;
        m_particles.push_back(p);
    }
}

void ParticleSystem::update(float dt) {
    for (size_t i = 0; i < m_particles.size(); ) {
        m_particles[i].life -= dt;
        if (m_particles[i].life <= 0.0f) {
            std::swap(m_particles[i], m_particles.back());
            m_particles.pop_back();
            continue;
        }
        m_particles[i].position.x += m_particles[i].velocity.x * dt;
        m_particles[i].position.y += m_particles[i].velocity.y * dt;
        m_particles[i].velocity.y += constants::GRAVITY * 0.3f * dt;
        ++i;
    }
}

void ParticleSystem::render() const {
    for (const auto& p : m_particles) {
        float t = p.life / p.maxLife;
        Color c = p.color;
        c.a = static_cast<unsigned char>(t * 255);
        DrawRectangle(static_cast<int>(p.position.x - p.size / 2),
                      static_cast<int>(p.position.y - p.size / 2),
                      static_cast<int>(p.size), static_cast<int>(p.size), c);
    }
}

void ParticleSystem::clear() {
    m_particles.clear();
}
