#include "MiningSystem.hpp"
#include "world/World.hpp"
#include "world/Tile.hpp"
#include "world/TileRegistry.hpp"
#include "items/Tool.hpp"
#include "entities/Player.hpp"
#include "items/Inventory.hpp"
#include "core/Math.hpp"

static bool hasAdjacentSolid(const World& world, int tileX, int tileY) {
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

void MiningSystem::tryMineTile(World& world, Player& player, int tileX, int tileY) {
    if (!world.isInBounds(tileX, tileY)) return;

    TileId tile = world.getTile(tileX, tileY);
    if (tile == TileId::Air) return;

    Vector2 playerCenter = {
        player.getPosition().x + player.getBounds().width / 2,
        player.getPosition().y + player.getBounds().height / 2
    };
    Vector2 tileCenter = {
        math::tileToWorldX(tileX) + constants::TILE_SIZE / 2.0f,
        math::tileToWorldY(tileY) + constants::TILE_SIZE / 2.0f
    };
    float dist = sqrtf((playerCenter.x - tileCenter.x) * (playerCenter.x - tileCenter.x) +
                       (playerCenter.y - tileCenter.y) * (playerCenter.y - tileCenter.y));
    if (dist > player.getReach()) return;

    auto* selected = player.getInventory().getSelectedSlot();
    TileId heldId = (selected && selected->count > 0) ? selected->tileId : TileId::Air;

    if (!isTool(heldId)) return;

    if (heldId == TileId::Axe) {
        if (isTreeTile(tile))
            fellTree(world, tileX, tileY, player.getInventory());
    } else if (heldId == TileId::Pickaxe) {
        if (isTreeTile(tile)) return;
        world.setTile(tileX, tileY, TileId::Air);
        player.getInventory().addItem(tile, 1);
    }
}

float MiningSystem::getMiningTime(TileId tile, TileId tool) {
    if (!isTool(tool)) return 0.0f;

    if (tool == TileId::Pickaxe) {
        switch (tile) {
            case TileId::Dirt:          return 0.15f;
            case TileId::Grass:         return 0.15f;
            case TileId::Sand:          return 0.15f;
            case TileId::Mud:           return 0.15f;
            case TileId::Clay:          return 0.20f;
            case TileId::Gravel:        return 0.20f;
            case TileId::SnowBlock:     return 0.15f;
            case TileId::Ice:           return 0.25f;
            case TileId::CopperOre:     return 0.30f;
            case TileId::IronOre:       return 0.40f;
            case TileId::GoldOre:       return 0.50f;
            case TileId::Stone:         return 0.35f;
            case TileId::Marble:        return 0.35f;
            case TileId::Granite:       return 0.40f;
            case TileId::MushroomGrass: return 0.15f;
            case TileId::JungleGrass:   return 0.15f;
            case TileId::Wood:          return 0.30f;
            case TileId::Planks:        return 0.30f;
            case TileId::Hellstone:     return 0.70f;
            case TileId::Torch:         return 0.05f;
            default:                    return 0.0f;
        }
    }

    if (tool == TileId::Axe) {
        switch (tile) {
            case TileId::Wood:          return 0.30f;
            case TileId::Leaf:          return 0.15f;
            case TileId::Cactus:        return 0.25f;
            default:                    return 0.0f;
        }
    }

    return 0.0f;
}

void MiningSystem::tryPlace(World& world, Player& player, const Camera2D& camera) {
    Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), camera);
    int tileX = math::worldToTileX(worldPos.x);
    int tileY = math::worldToTileY(worldPos.y);

    if (!world.isInBounds(tileX, tileY))
        return;

    if (world.getTile(tileX, tileY) != TileId::Air)
        return;

    Rectangle playerRect = player.getBounds();
    Rectangle tileRect = {
        math::tileToWorldX(tileX), math::tileToWorldY(tileY),
        static_cast<float>(constants::TILE_SIZE), static_cast<float>(constants::TILE_SIZE)
    };
    if (CheckCollisionRecs(playerRect, tileRect))
        return;

    auto* selected = player.getInventory().getSelectedSlot();
    if (!selected || selected->tileId == TileId::Air || selected->count <= 0)
        return;

    if (isTool(selected->tileId))
        return;

    if (!hasAdjacentSolid(world, tileX, tileY))
        return;

    world.setTile(tileX, tileY, selected->tileId);
    player.getInventory().removeItem(selected->tileId, 1);
}
