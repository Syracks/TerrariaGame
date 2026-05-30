#pragma once

#include "world/Tile.hpp"
#include <vector>

struct Ingredient {
    TileId item;
    int count;
};

enum class CraftingStation {
    None,
    Workbench,
    Furnace,
    Anvil
};

struct CraftingStationDef {
    TileId stationTile;
    float range;
};

struct Recipe {
    TileId result;
    int resultCount;
    std::vector<Ingredient> ingredients;
    CraftingStation station = CraftingStation::None;
};

inline CraftingStationDef getStationDef(CraftingStation s) {
    switch (s) {
        case CraftingStation::Workbench: return {TileId::Workbench, 3.0f};
        case CraftingStation::Furnace:   return {TileId::Furnace,   3.0f};
        case CraftingStation::Anvil:     return {TileId::Anvil,     3.0f};
        default:                         return {TileId::Air,       0.0f};
    }
}

class RecipeDatabase {
public:
    static const std::vector<Recipe>& getAll();

    template<typename SlotIter>
    static std::vector<const Recipe*> getAvailable(SlotIter begin, SlotIter end,
                                                    CraftingStation station = CraftingStation::None) {
        std::vector<const Recipe*> available;
        for (const auto& recipe : getAll()) {
            if (recipe.station != station) continue;
            bool canCraft = true;
            for (const auto& ing : recipe.ingredients) {
                int total = 0;
                for (auto it = begin; it != end; ++it) {
                    if (it->tileId == ing.item) total += it->count;
                }
                if (total < ing.count) { canCraft = false; break; }
            }
            if (canCraft) available.push_back(&recipe);
        }
        return available;
    }
};
