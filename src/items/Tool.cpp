#include "Tool.hpp"
#include "world/World.hpp"
#include "items/Inventory.hpp"

#include <vector>
#include <utility>

bool isTool(TileId id) {
    return id == TileId::Pickaxe || id == TileId::Axe || id == TileId::Sword;
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

    while (!stack.empty()) {
        auto [x, y] = stack.back();
        stack.pop_back();

        if (!world.isInBounds(x, y))
            continue;

        TileId t = world.getTile(x, y);
        if (!isTreeTile(t))
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

        stack.push_back({x, y - 1});
        stack.push_back({x - 1, y});
        stack.push_back({x + 1, y});
        if (t == TileId::Leaf)
            stack.push_back({x, y + 1});
    }

    for (auto& p : visited)
        world.setTile(p.first, p.second, TileId::Air);

    if (woodCount > 0)
        inventory.addItem(TileId::Planks, woodCount);
}
