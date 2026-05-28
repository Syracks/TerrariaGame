#pragma once

#include <raylib.h>
#include <vector>

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float life;
    float maxLife;
    float size;
};

class ParticleSystem {
public:
    void emit(Vector2 pos, Vector2 vel, Color color, float life, float size, int count);
    void update(float dt);
    void render() const;
    void clear();

private:
    std::vector<Particle> m_particles;
};
