#include "Recipe.hpp"

const std::vector<Recipe>& RecipeDatabase::getAll() {
    static const std::vector<Recipe> recipes = {
        {TileId::Planks, 4, {{TileId::Wood, 1}}},
        {TileId::Pickaxe, 1, {{TileId::Stone, 3}, {TileId::Wood, 2}}},
        {TileId::Axe, 1, {{TileId::Stone, 3}, {TileId::Wood, 2}}},
        {TileId::Sword, 1, {{TileId::Stone, 3}, {TileId::Wood, 2}}},
        {TileId::Torch, 3, {{TileId::Planks, 1}}},
        {TileId::DirtWall, 4, {{TileId::Dirt, 1}}},
        {TileId::StoneWall, 4, {{TileId::Stone, 1}}},
        {TileId::WoodWall, 4, {{TileId::Wood, 1}}},
        {TileId::PlanksWall, 4, {{TileId::Planks, 1}}},
    };
    return recipes;
}
