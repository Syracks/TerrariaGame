#pragma once

#include <raylib.h>
#include <cstdint>

enum class ProjectileType : uint8_t {
    Arrow,
    Leaf,
    Fireball
};

struct Projectile {
    Vector2 position{};
    Vector2 velocity{};
    float lifetime = 2.0f;
    bool active = true;
    int facing = 1;
    ProjectileType type = ProjectileType::Arrow;
    int damage = 10;
    bool fromBoss = false;
    bool noGravity = false;
};
