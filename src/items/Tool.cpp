#include "Tool.hpp"
#include "world/World.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDefinition.hpp"

#include <vector>
#include <utility>

bool isTool(TileId id) {
    return id == TileId::Pickaxe || id == TileId::Axe || id == TileId::Sword ||
           id == TileId::CopperPickaxe || id == TileId::IronPickaxe || id == TileId::GoldPickaxe ||
           id == TileId::CopperAxe || id == TileId::IronAxe || id == TileId::GoldAxe ||
           id == TileId::CopperSword || id == TileId::IronSword || id == TileId::GoldSword ||
           id == TileId::Hammer || id == TileId::CopperHammer ||
           id == TileId::IronHammer || id == TileId::GoldHammer;
}

bool isPickaxe(TileId id) {
    return id == TileId::Pickaxe || id == TileId::CopperPickaxe ||
           id == TileId::IronPickaxe || id == TileId::GoldPickaxe;
}

bool isAxe(TileId id) {
    return id == TileId::Axe || id == TileId::CopperAxe ||
           id == TileId::IronAxe || id == TileId::GoldAxe;
}

bool isSword(TileId id) {
    return id == TileId::Sword || id == TileId::CopperSword ||
           id == TileId::IronSword || id == TileId::GoldSword;
}

int getToolMiningLevel(TileId id) {
    return ItemDatabase::instance().get(id).tool.miningLevel;
}

bool isHammer(TileId id) {
    return id == TileId::Hammer || id == TileId::CopperHammer ||
           id == TileId::IronHammer || id == TileId::GoldHammer;
}

bool isBow(TileId id) {
    return id == TileId::CopperBow || id == TileId::IronBow || id == TileId::GoldBow;
}

bool isTreeTile(TileId id) {
    return id == TileId::Wood || id == TileId::Leaf;
}

bool isWallItem(TileId id) {
    return id == TileId::DirtWall || id == TileId::StoneWall ||
           id == TileId::WoodWall || id == TileId::PlanksWall;
}

void fellTree(World& world, int tileX, int tileY, Inventory& inventory) {
    std::vector<std::pair<int, int>> stack;
    std::vector<std::pair<int, int>> visited;
    stack.push_back({tileX, tileY});

    int woodCount = 0;
    int cactusCount = 0;

    while (!stack.empty()) {
        auto [x, y] = stack.back();
        stack.pop_back();

        if (!world.isInBounds(x, y))
            continue;

        TileId t = world.getTile(x, y);
        if (!isTreeTile(t) && t != TileId::Cactus)
            continue;

        bool already = false;
        for (auto& p : visited) {
            if (p.first == x && p.second == y) {
                already = true;
                break;
            }
        }
        if (already)
            continue;

        visited.push_back({x, y});

        if (t == TileId::Wood)
            woodCount++;
        else if (t == TileId::Cactus)
            cactusCount++;

        if (t == TileId::Cactus) {
            stack.push_back({x, y - 1});
        } else {
            stack.push_back({x, y - 1});
            stack.push_back({x - 1, y});
            stack.push_back({x + 1, y});
            if (t == TileId::Leaf)
                stack.push_back({x, y + 1});
        }
    }

    for (auto& p : visited)
        world.setTile(p.first, p.second, TileId::Air);

    if (woodCount > 0)
        inventory.addItem(TileId::Planks, woodCount * 4);
    if (cactusCount > 0)
        inventory.addItem(TileId::Cactus, cactusCount);
}
