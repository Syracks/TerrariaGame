#include "InteractionSystem.hpp"
#include "entities/Player.hpp"
#include "world/World.hpp"
#include "world/TileRegistry.hpp"
#include "systems/ParticleSystem.hpp"
#include "systems/MiningSystem.hpp"
#include "ui/Minimap.hpp"
#include "items/Tool.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDefinition.hpp"
#include "core/Input.hpp"
#include "core/Math.hpp"
#include "core/Constants.hpp"
#include "core/SoundManager.hpp"

bool hasAdjacentSolidForInteraction(const World& world, int tileX, int tileY) {
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        int nx = tileX + dx[i];
        int ny = tileY + dy[i];
        if (world.isInBounds(nx, ny)) {
            TileId nid = world.getTile(nx, ny);
            if (nid != TileId::Air && TileRegistry::instance().get(nid).solid)
                return true;
        }
    }
    return false;
}

bool hasAdjacentWall(const World& world, int tileX, int tileY) {
    static const int dx[] = {0, 0, -1, 1};
    static const int dy[] = {-1, 1, 0, 0};
    for (int i = 0; i < 4; ++i) {
        int nx = tileX + dx[i];
        int ny = tileY + dy[i];
        if (world.isInBounds(nx, ny)) {
            if (world.getWall(nx, ny) != TileId::Air)
                return true;
        }
    }
    return false;
}

void InteractionSystem::handleMinePress(Player& player, const World& world,
                                         const Camera2D& camera) {
    auto* sel = player.getInventory().getSelectedSlot();
    TileId held = (sel && sel->count > 0) ? sel->tileId : TileId::Air;

    if (isSword(held)) {
        player.startSwing();
        SoundManager::instance().play(SoundManager::SwordSwing);
        return;
    }

    if (!isTool(held)) return;

    Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    int tx = math::worldToTileX(worldPos.x);
    int ty = math::worldToTileY(worldPos.y);
    if (!world.isInBounds(tx, ty)) return;

    TileId target = world.getTile(tx, ty);
    if (target != TileId::Air) {
        float mineTime = MiningSystem::getMiningTime(target, held);
        if (mineTime > 0.0f) {
            player.setMiningTarget(tx, ty);
            player.startSwing(mineTime);
        }
    } else if (isHammer(held) && world.getWall(tx, ty) != TileId::Air) {
        player.setMiningTarget(tx, ty);
        player.startSwing(0.15f);
    }
}

void InteractionSystem::handleSwingCompletion(Player& player, World& world,
                                               ParticleSystem& particles,
                                               Minimap& minimap) {
    if (!player.wasSwingJustCompleted()) return;

    auto* sel = player.getInventory().getSelectedSlot();
    TileId held = (sel && sel->count > 0) ? sel->tileId : TileId::Air;

    if ((isPickaxe(held) || isAxe(held) || isHammer(held)) &&
        player.getMiningTargetX() >= 0) {
        int tx = player.getMiningTargetX();
        int ty = player.getMiningTargetY();
        TileId mined = world.getTile(tx, ty);

        if (mined != TileId::Air) {
            MiningSystem::tryMineTile(world, player, tx, ty);
            if (world.getTile(tx, ty) == TileId::Air && mined != TileId::Air) {
                SoundManager::instance().play(
                    isPickaxe(held) ? SoundManager::PickaxeMine : SoundManager::AxeMine);
                float cx = math::tileToWorldX(tx) + constants::TILE_SIZE / 2.0f;
                float cy = math::tileToWorldY(ty) + constants::TILE_SIZE / 2.0f;
                auto& reg = TileRegistry::instance();
                Color c = reg.get(mined).color;
                for (int i = 0; i < 6; ++i) {
                    particles.emit({cx, cy}, {0, -150}, c, 0.5f, 4, 1);
                }
                minimap.markDirty();
            }
        } else {
            TileId wall = world.getWall(tx, ty);
            if (wall != TileId::Air && isHammer(held)) {
                world.setWall(tx, ty, TileId::Air);
                player.getInventory().addItem(wall, 1);
                SoundManager::instance().play(SoundManager::PickaxeMine);
                float cx = math::tileToWorldX(tx) + constants::TILE_SIZE / 2.0f;
                float cy = math::tileToWorldY(ty) + constants::TILE_SIZE / 2.0f;
                auto& reg = TileRegistry::instance();
                Color c = reg.get(wall).color;
                for (int i = 0; i < 4; ++i) {
                    particles.emit({cx, cy}, {0, -120}, c, 0.4f, 3, 1);
                }
                minimap.markDirty();
            }
        }
        player.clearMiningTarget();
    }
}

void InteractionSystem::handlePlacePress(Player& player, World& world,
                                          const Camera2D& camera,
                                          Minimap& minimap) {
    Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    int tx = math::worldToTileX(worldPos.x);
    int ty = math::worldToTileY(worldPos.y);

    if (world.isInBounds(tx, ty) && world.getTile(tx, ty) == TileId::Door) {
        bool open = world.isDoorOpen(tx, ty);
        world.setDoorOpen(tx, ty, !open);
        if (world.isInBounds(tx, ty - 1) && world.getTile(tx, ty - 1) == TileId::Door)
            world.setDoorOpen(tx, ty - 1, !open);
        if (world.isInBounds(tx, ty + 1) && world.getTile(tx, ty + 1) == TileId::Door)
            world.setDoorOpen(tx, ty + 1, !open);
        SoundManager::instance().play(SoundManager::BlockPlace);
        minimap.markDirty();
        return;
    }

    auto* sel = player.getInventory().getSelectedSlot();
    if (!sel || sel->count <= 0) return;

    if (sel->tileId == TileId::Door) {
        if (world.isInBounds(tx, ty) && world.getTile(tx, ty) == TileId::Air &&
            world.isInBounds(tx, ty - 1) && world.getTile(tx, ty - 1) == TileId::Air &&
            hasAdjacentSolidForInteraction(world, tx, ty)) {
            Rectangle playerRect = player.getBounds();
            Rectangle doorRect = {
                math::tileToWorldX(tx), math::tileToWorldY(ty - 1),
                static_cast<float>(constants::TILE_SIZE), static_cast<float>(constants::TILE_SIZE) * 2
            };
            if (!CheckCollisionRecs(playerRect, doorRect)) {
                world.setTile(tx, ty, TileId::Door);
                world.setTile(tx, ty - 1, TileId::Door);
                player.getInventory().removeItem(TileId::Door, 1);
                SoundManager::instance().play(SoundManager::BlockPlace);
                minimap.markDirty();
            }
        }
    } else if (sel->tileId == TileId::Torch) {
        if (world.isInBounds(tx, ty) && world.getTile(tx, ty) == TileId::Air) {
            if (hasAdjacentSolidForInteraction(world, tx, ty)) {
                world.setTile(tx, ty, TileId::Torch);
                player.getInventory().removeItem(TileId::Torch, 1);
                SoundManager::instance().play(SoundManager::TorchPlace);
                minimap.markDirty();
            }
        }
    } else if (isWallItem(sel->tileId)) {
        if (world.isInBounds(tx, ty)) {
            if (world.getWall(tx, ty) == TileId::Air &&
                (hasAdjacentSolidForInteraction(world, tx, ty) || hasAdjacentWall(world, tx, ty) ||
                 world.getTile(tx, ty) != TileId::Air)) {
                world.setWall(tx, ty, sel->tileId);
                player.getInventory().removeItem(sel->tileId, 1);
                SoundManager::instance().play(SoundManager::BlockPlace);
                minimap.markDirty();
            }
        }
    } else if (sel->tileId == TileId::WoodenTable) {
        if (world.isInBounds(tx, ty) && world.getTile(tx, ty) == TileId::Air &&
            world.isInBounds(tx+1, ty) && world.getTile(tx+1, ty) == TileId::Air &&
            world.isInBounds(tx, ty+1) && world.getTile(tx, ty+1) == TileId::Air &&
            world.isInBounds(tx+1, ty+1) && world.getTile(tx+1, ty+1) == TileId::Air) {
            Rectangle playerRect = player.getBounds();
            Rectangle tableRect = {
                math::tileToWorldX(tx), math::tileToWorldY(ty),
                static_cast<float>(constants::TILE_SIZE) * 2, static_cast<float>(constants::TILE_SIZE) * 2
            };
            if (!CheckCollisionRecs(playerRect, tableRect)) {
                bool hasAdj = false;
                for (int dx = -1; dx <= 2 && !hasAdj; ++dx) {
                    for (int dy = -1; dy <= 2 && !hasAdj; ++dy) {
                        if ((dx == -1 || dx == 2 || dy == -1 || dy == 2) &&
                            world.isInBounds(tx + dx, ty + dy)) {
                            TileId nid = world.getTile(tx + dx, ty + dy);
                            if (nid != TileId::Air && TileRegistry::instance().get(nid).solid)
                                hasAdj = true;
                        }
                    }
                }
                if (hasAdj) {
                    world.setTile(tx, ty, TileId::WoodenTable);
                    world.setTile(tx+1, ty, TileId::WoodenTable);
                    world.setTile(tx, ty+1, TileId::WoodenTable);
                    world.setTile(tx+1, ty+1, TileId::WoodenTable);
                    player.getInventory().removeItem(TileId::WoodenTable, 1);
                    SoundManager::instance().play(SoundManager::BlockPlace);
                    minimap.markDirty();
                }
            }
        }
        return;
    } else if (sel->tileId == TileId::Workbench ||
               sel->tileId == TileId::Furnace ||
               sel->tileId == TileId::Anvil ||
               sel->tileId == TileId::ChestBlock) {
        if (world.isInBounds(tx, ty) && world.getTile(tx, ty) == TileId::Air) {
            Rectangle playerRect = player.getBounds();
            Rectangle tileRect = {
                math::tileToWorldX(tx), math::tileToWorldY(ty),
                static_cast<float>(constants::TILE_SIZE), static_cast<float>(constants::TILE_SIZE)
            };
            if (!CheckCollisionRecs(playerRect, tileRect) &&
                hasAdjacentSolidForInteraction(world, tx, ty)) {
                world.setTile(tx, ty, sel->tileId);
                player.getInventory().removeItem(sel->tileId, 1);
                SoundManager::instance().play(SoundManager::BlockPlace);
                minimap.markDirty();
            }
        }
    } else {
        int oldCount = sel->count;
        MiningSystem::tryPlace(world, player, camera);
        if (sel->count < oldCount) {
            SoundManager::instance().play(SoundManager::BlockPlace);
            minimap.markDirty();
        }
    }
}

void InteractionSystem::handleChestInteraction(Player& player, World& world,
                                                const Camera2D& camera) {
    Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    int tx = math::worldToTileX(worldPos.x);
    int ty = math::worldToTileY(worldPos.y);
    if (!world.isInBounds(tx, ty)) return;
    if (world.getTile(tx, ty) != TileId::ChestBlock) return;

    float distX = std::abs(player.getPosition().x + player.getBounds().width / 2 -
                           (math::tileToWorldX(tx) + constants::TILE_SIZE / 2.0f));
    float distY = std::abs(player.getPosition().y + player.getBounds().height / 2 -
                           (math::tileToWorldY(ty) + constants::TILE_SIZE / 2.0f));
    if (distX > 3.0f * constants::TILE_SIZE || distY > 3.0f * constants::TILE_SIZE) return;

    SoundManager::instance().play(SoundManager::BlockPlace);
}
