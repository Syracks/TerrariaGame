#include "ParticleSystem.hpp"
#include "core/Constants.hpp"

#include <random>
#include <cmath>

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
    for (auto it = m_particles.begin(); it != m_particles.end(); ) {
        it->life -= dt;
        if (it->life <= 0.0f) {
            it = m_particles.erase(it);
            continue;
        }
        it->position.x += it->velocity.x * dt;
        it->position.y += it->velocity.y * dt;
        it->velocity.y += constants::GRAVITY * 0.3f * dt;
        ++it;
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
