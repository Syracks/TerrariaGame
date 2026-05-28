#include "CollisionSystem.hpp"
#include "world/World.hpp"
#include "world/Tile.hpp"
#include "entities/Entity.hpp"
#include "core/Math.hpp"
#include "core/Constants.hpp"

bool CollisionSystem::isTileSolidAt(const World& world, float worldX, float worldY) {
    int tileX = math::worldToTileX(worldX);
    int tileY = math::worldToTileY(worldY);
    return world.isSolid(tileX, tileY);
}

bool CollisionSystem::isRectSolid(const World& world, Rectangle rect) {
    int startX = math::worldToTileX(rect.x);
    int startY = math::worldToTileY(rect.y);
    int endX = math::worldToTileX(rect.x + rect.width - 0.01f);
    int endY = math::worldToTileY(rect.y + rect.height - 0.01f);

    for (int ty = startY; ty <= endY; ++ty) {
        for (int tx = startX; tx <= endX; ++tx) {
            if (world.isSolid(tx, ty))
                return true;
        }
    }
    return false;
}

bool CollisionSystem::resolveCollision(Entity& entity, const World& world, float dt) {
    Vector2 pos = entity.getPosition();
    Vector2 vel = entity.getVelocity();
    Rectangle bounds = entity.getBounds();

    float newX = pos.x + vel.x * dt;
    if (vel.x > 0.0f) {
        float edgeX = newX + bounds.width - 0.01f;
        int tileX = math::worldToTileX(edgeX);
        int startY = math::worldToTileY(pos.y);
        int endY = math::worldToTileY(pos.y + bounds.height - 0.01f);
        for (int ty = startY; ty <= endY; ++ty) {
            if (world.isSolid(tileX, ty)) {
                newX = static_cast<float>(tileX) * constants::TILE_SIZE - bounds.width;
                vel.x = 0.0f;
                break;
            }
        }
    } else if (vel.x < 0.0f) {
        float edgeX = newX;
        int tileX = math::worldToTileX(edgeX);
        int startY = math::worldToTileY(pos.y);
        int endY = math::worldToTileY(pos.y + bounds.height - 0.01f);
        for (int ty = startY; ty <= endY; ++ty) {
            if (world.isSolid(tileX, ty)) {
                newX = static_cast<float>(tileX + 1) * constants::TILE_SIZE;
                vel.x = 0.0f;
                break;
            }
        }
    }
    pos.x = newX;

    float newY = pos.y + vel.y * dt;
    if (vel.y > 0.0f) {
        float edgeY = newY + bounds.height - 0.01f;
        int tileY = math::worldToTileY(edgeY);
        int startX = math::worldToTileX(pos.x);
        int endX = math::worldToTileX(pos.x + bounds.width - 0.01f);
        bool hitGround = false;
        for (int tx = startX; tx <= endX; ++tx) {
            if (world.isSolid(tx, tileY)) {
                newY = static_cast<float>(tileY) * constants::TILE_SIZE - bounds.height;
                hitGround = true;
                break;
            }
        }
        if (hitGround) {
            entity.setOnGround(true);
            vel.y = 0.0f;
        } else {
            entity.setOnGround(false);
            pos.y = newY;
        }
    } else if (vel.y < 0.0f) {
        float edgeY = newY;
        int tileY = math::worldToTileY(edgeY);
        int startX = math::worldToTileX(pos.x);
        int endX = math::worldToTileX(pos.x + bounds.width - 0.01f);
        for (int tx = startX; tx <= endX; ++tx) {
            if (world.isSolid(tx, tileY)) {
                newY = static_cast<float>(tileY + 1) * constants::TILE_SIZE;
                vel.y = 0.0f;
                break;
            }
        }
        pos.y = newY;
        entity.setOnGround(false);
    } else {
        pos.y = newY;
        entity.setOnGround(false);
    }

    entity.setPosition(pos);
    entity.setVelocity(vel);
    return entity.isOnGround();
}
