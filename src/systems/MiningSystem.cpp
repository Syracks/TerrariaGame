#include "MiningSystem.hpp"
#include "world/World.hpp"
#include "world/Tile.hpp"
#include "world/TileRegistry.hpp"
#include "items/Tool.hpp"
#include "items/ItemDefinition.hpp"
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

    if (isAxe(heldId)) {
        if (isTreeTile(tile) || tile == TileId::Cactus) {
            fellTree(world, tileX, tileY, player.getInventory());
        } else {
            return;
        }
    } else if (isPickaxe(heldId)) {
        if (isTreeTile(tile)) return;

        int toolLevel = getToolMiningLevel(heldId);
        int requiredLevel = 0;
        switch (tile) {
            case TileId::CopperOre: requiredLevel = 1; break;
            case TileId::IronOre:   requiredLevel = 1; break;
            case TileId::GoldOre:   requiredLevel = 2; break;
            case TileId::Hellstone: requiredLevel = 3; break;
            case TileId::Granite:   requiredLevel = 1; break;
            case TileId::Marble:    requiredLevel = 1; break;
            default:                requiredLevel = 0; break;
        }

        if (toolLevel < requiredLevel) return;

        if (tile == TileId::ChestBlock) {
            const auto& chestInv = world.getChestConst(tileX, tileY);
            bool empty = true;
            for (const auto& slot : chestInv) {
                if (slot.tileId != TileId::Air && slot.count > 0) {
                    empty = false;
                    break;
                }
            }
            if (!empty) return;
        }

        world.setTile(tileX, tileY, TileId::Air);
        player.getInventory().addItem(tile, 1);

        if (tile == TileId::ChestBlock) {
            world.removeChest(tileX, tileY);
        } else if (tile == TileId::WoodenTable) {
            int tlX = tileX, tlY = tileY;
            if (world.isInBounds(tileX-1, tileY) && world.getTile(tileX-1, tileY) == TileId::WoodenTable)
                tlX--;
            if (world.isInBounds(tileX, tileY-1) && world.getTile(tileX, tileY-1) == TileId::WoodenTable)
                tlY--;
            if (world.isInBounds(tlX+1, tlY) && world.getTile(tlX+1, tlY) == TileId::WoodenTable &&
                world.isInBounds(tlX, tlY+1) && world.getTile(tlX, tlY+1) == TileId::WoodenTable &&
                world.isInBounds(tlX+1, tlY+1) && world.getTile(tlX+1, tlY+1) == TileId::WoodenTable) {
                world.setTile(tlX+1, tlY, TileId::Air);
                world.setTile(tlX, tlY+1, TileId::Air);
                world.setTile(tlX+1, tlY+1, TileId::Air);
            }
        } else if (tile == TileId::Door) {
            if (world.isInBounds(tileX, tileY - 1) && world.getTile(tileX, tileY - 1) == TileId::Door)
                world.setTile(tileX, tileY - 1, TileId::Air);
            if (world.isInBounds(tileX, tileY + 1) && world.getTile(tileX, tileY + 1) == TileId::Door)
                world.setTile(tileX, tileY + 1, TileId::Air);
        }
    }
}

float MiningSystem::getMiningTime(TileId tile, TileId tool) {
    if (!isTool(tool)) return 0.0f;

    float speedMult = ItemDatabase::instance().get(tool).tool.miningSpeed;

    if (isPickaxe(tool)) {
        int toolLevel = getToolMiningLevel(tool);
        int requiredLevel = 0;
        float baseTime = 0.0f;

        switch (tile) {
            case TileId::Dirt:          baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::Grass:         baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::Sand:          baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::Mud:           baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::Clay:          baseTime = 0.20f; requiredLevel = 0; break;
            case TileId::Gravel:        baseTime = 0.20f; requiredLevel = 0; break;
            case TileId::SnowBlock:     baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::Ice:           baseTime = 0.25f; requiredLevel = 0; break;
            case TileId::CopperOre:     baseTime = 0.40f; requiredLevel = 1; break;
            case TileId::IronOre:       baseTime = 0.50f; requiredLevel = 1; break;
            case TileId::GoldOre:       baseTime = 0.65f; requiredLevel = 2; break;
            case TileId::Stone:         baseTime = 0.40f; requiredLevel = 0; break;
            case TileId::Marble:        baseTime = 0.40f; requiredLevel = 1; break;
            case TileId::Granite:       baseTime = 0.45f; requiredLevel = 1; break;
            case TileId::MushroomGrass: baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::JungleGrass:   baseTime = 0.15f; requiredLevel = 0; break;
            case TileId::Wood:          baseTime = 0.40f; requiredLevel = 0; break;
            case TileId::Planks:        baseTime = 0.30f; requiredLevel = 0; break;
            case TileId::Hellstone:     baseTime = 0.90f; requiredLevel = 3; break;
            case TileId::Torch:         baseTime = 0.05f; requiredLevel = 0; break;
            case TileId::Door:          baseTime = 0.30f; requiredLevel = 0; break;
            case TileId::Workbench:     baseTime = 0.40f; requiredLevel = 0; break;
            case TileId::Furnace:       baseTime = 0.50f; requiredLevel = 0; break;
            case TileId::Anvil:         baseTime = 0.40f; requiredLevel = 0; break;
            case TileId::ChestBlock:    baseTime = 0.40f; requiredLevel = 0; break;
            case TileId::WoodenChair:   baseTime = 0.30f; requiredLevel = 0; break;
            case TileId::WoodenTable:   baseTime = 0.35f; requiredLevel = 0; break;
            default:                    return 0.0f;
        }

        if (toolLevel < requiredLevel) return 0.0f;

        return baseTime / speedMult;
    }

    if (isAxe(tool)) {
        float speedMult = ItemDatabase::instance().get(tool).tool.miningSpeed;
        switch (tile) {
            case TileId::Wood:          return 0.80f / speedMult;
            case TileId::Leaf:          return 0.40f / speedMult;
            case TileId::Cactus:        return 0.60f / speedMult;
            default:                    return 0.0f;
        }
    }

    return 0.0f;
}

void MiningSystem::tryPlace(World& world, Player& player, const Camera2D& camera) {
    Vector2 worldPos = GetScreenToWorld2D(math::getVirtualMouse(), camera);
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

    if (!ItemDatabase::instance().get(selected->tileId).placeable)
        return;

    if (!hasAdjacentSolid(world, tileX, tileY))
        return;

    world.setTile(tileX, tileY, selected->tileId);
    player.getInventory().removeItem(selected->tileId, 1);
}
