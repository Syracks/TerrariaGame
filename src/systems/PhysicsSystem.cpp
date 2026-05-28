#include "PhysicsSystem.hpp"
#include "CollisionSystem.hpp"
#include "entities/Entity.hpp"
#include "world/World.hpp"
#include "core/Constants.hpp"

void PhysicsSystem::update(Entity& entity, const World& world, float dt) {
    Vector2 vel = entity.getVelocity();
    vel.y += constants::GRAVITY * dt;
    entity.setVelocity(vel);
    CollisionSystem::resolveCollision(entity, world, dt);
}
