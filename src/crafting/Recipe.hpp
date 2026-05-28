#pragma once

#include "world/Tile.hpp"
#include <vector>

struct Ingredient {
    TileId item;
    int count;
};

struct Recipe {
    TileId result;
    int resultCount;
    std::vector<Ingredient> ingredients;
};

class RecipeDatabase {
public:
    static const std::vector<Recipe>& getAll();

    template<typename SlotIter>
    static std::vector<const Recipe*> getAvailable(SlotIter begin, SlotIter end) {
        std::vector<const Recipe*> available;
        for (const auto& recipe : getAll()) {
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
