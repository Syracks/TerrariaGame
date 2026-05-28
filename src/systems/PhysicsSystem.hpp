#pragma once

class Entity;
class World;

class PhysicsSystem {
public:
    static void update(Entity& entity, const World& world, float dt);
};
