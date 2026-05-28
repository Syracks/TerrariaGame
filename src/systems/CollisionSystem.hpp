#pragma once

#include <raylib.h>

class World;
class Entity;

class CollisionSystem {
public:
    static bool resolveCollision(Entity& entity, const World& world, float dt);
    static bool isTileSolidAt(const World& world, float worldX, float worldY);
    static bool isRectSolid(const World& world, Rectangle rect);
};
