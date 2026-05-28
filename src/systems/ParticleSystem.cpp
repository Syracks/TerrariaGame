#include "ParticleSystem.hpp"
#include "core/Constants.hpp"

#include <cstdlib>
#include <cmath>

void ParticleSystem::emit(Vector2 pos, Vector2 vel, Color color, float life, float size, int count) {
    for (int i = 0; i < count; ++i) {
        Particle p;
        p.position = pos;
        p.velocity = {
            vel.x + static_cast<float>(std::rand() % 100 - 50) * 0.5f,
            vel.y + static_cast<float>(std::rand() % 100 - 50) * 0.5f
        };
        p.color = color;
        p.life = life + static_cast<float>(std::rand() % 100) / 100.0f * life * 0.5f;
        p.maxLife = p.life;
        p.size = size + static_cast<float>(std::rand() % 100) / 100.0f * size;
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
