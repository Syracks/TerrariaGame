#include "PhysicsSystem.hpp"
#include "CollisionSystem.hpp"
#include "LiquidSystem.hpp"
#include "entities/Entity.hpp"
#include "world/World.hpp"
#include "core/Constants.hpp"

void PhysicsSystem::update(Entity& entity, const World& world, float dt) {
    Vector2 vel = entity.getVelocity();

    bool inWater = LiquidSystem::isInWater(world, entity.getPosition().x, entity.getPosition().y,
                                             entity.getBounds().width, entity.getBounds().height);
    if (inWater) {
        vel.y += constants::GRAVITY * 0.4f * dt;
        vel.x *= 0.90f;
    } else {
        vel.y += constants::GRAVITY * dt;
    }

    entity.setVelocity(vel);
    CollisionSystem::resolveCollision(entity, world, dt);
}
