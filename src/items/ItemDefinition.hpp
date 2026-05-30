#pragma once

#include "world/Tile.hpp"
#include <string>
#include <unordered_map>

struct ToolStats {
    float miningSpeed = 1.0f;
    int miningLevel = 0;
    int damage = 0;
};

struct ItemDefinition {
    TileId id = TileId::Air;
    std::string name;
    int maxStack = 999;
    bool placeable = false;
    bool consumable = false;
    ToolStats tool{};
};

class ItemDatabase {
public:
    static ItemDatabase& instance();

    const ItemDefinition& get(TileId id) const;
    int getMaxStack(TileId id) const;

private:
    ItemDatabase();
    std::unordered_map<TileId, ItemDefinition> m_items;
};
