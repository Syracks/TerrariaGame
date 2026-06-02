#include "Recipe.hpp"

const std::vector<Recipe>& RecipeDatabase::getAll() {
    using enum CraftingStation;

    static const std::vector<Recipe> recipes = {
        {TileId::Torch, 3, {{TileId::Planks, 1}}, None},
        {TileId::Torch, 5, {{TileId::Planks, 1}, {TileId::Gel, 1}}, None},
        {TileId::Planks, 1, {{TileId::Wood, 1}}, None},
        {TileId::DirtWall, 4, {{TileId::Dirt, 1}}, None},
        {TileId::StoneWall, 4, {{TileId::Stone, 1}}, None},
        {TileId::PlanksWall, 4, {{TileId::Planks, 1}}, None},

        {TileId::Workbench, 1, {{TileId::Planks, 20}}, None},

        {TileId::Furnace, 1, {{TileId::Stone, 20}, {TileId::Torch, 5}}, Workbench},
        {TileId::Anvil, 1, {{TileId::IronBar, 10}}, Workbench},
        {TileId::ChestBlock, 1, {{TileId::Planks, 8}, {TileId::IronBar, 2}}, Workbench},
        {TileId::Wood, 2, {{TileId::Planks, 1}}, Workbench},
        {TileId::PlanksWall, 8, {{TileId::Planks, 1}}, Workbench},
        {TileId::WoodWall, 4, {{TileId::Wood, 1}}, None},
        {TileId::Pickaxe, 1, {{TileId::Wood, 6}, {TileId::Stone, 4}}, Workbench},
        {TileId::Axe, 1, {{TileId::Wood, 6}, {TileId::Stone, 4}}, Workbench},
        {TileId::Sword, 1, {{TileId::Wood, 5}, {TileId::Stone, 3}}, Workbench},
        {TileId::Hammer, 1, {{TileId::Wood, 6}, {TileId::Stone, 4}}, Workbench},
        {TileId::Door, 1, {{TileId::Planks, 6}}, Workbench},
        {TileId::WoodenChair, 1, {{TileId::Planks, 4}}, Workbench},
        {TileId::WoodenTable, 1, {{TileId::Planks, 8}}, Workbench},

        {TileId::CopperBar, 1, {{TileId::CopperOre, 3}}, Furnace},
        {TileId::IronBar, 1, {{TileId::IronOre, 3}}, Furnace},
        {TileId::GoldBar, 1, {{TileId::GoldOre, 4}}, Furnace},

        {TileId::CopperPickaxe, 1, {{TileId::CopperBar, 20}, {TileId::Planks, 10}}, Anvil},
        {TileId::CopperAxe, 1,     {{TileId::CopperBar, 15}, {TileId::Planks, 10}}, Anvil},
        {TileId::CopperSword, 1,   {{TileId::CopperBar, 10}, {TileId::Planks, 10}}, Anvil},
        {TileId::CopperHammer, 1,  {{TileId::CopperBar, 10}, {TileId::Planks, 10}}, Anvil},

        {TileId::IronPickaxe, 1, {{TileId::IronBar, 20}, {TileId::Planks, 10}}, Anvil},
        {TileId::IronAxe, 1,     {{TileId::IronBar, 15}, {TileId::Planks, 10}}, Anvil},
        {TileId::IronSword, 1,   {{TileId::IronBar, 10}, {TileId::Planks, 10}}, Anvil},
        {TileId::IronHammer, 1,  {{TileId::IronBar, 10}, {TileId::Planks, 10}}, Anvil},

        {TileId::GoldPickaxe, 1, {{TileId::GoldBar, 20}, {TileId::Planks, 10}}, Anvil},
        {TileId::GoldAxe, 1,     {{TileId::GoldBar, 15}, {TileId::Planks, 10}}, Anvil},
        {TileId::GoldSword, 1,   {{TileId::GoldBar, 10}, {TileId::Planks, 10}}, Anvil},
        {TileId::GoldHammer, 1,  {{TileId::GoldBar, 10}, {TileId::Planks, 10}}, Anvil},

        {TileId::CopperBow, 1, {{TileId::CopperBar, 8}, {TileId::Gel, 3}}, Anvil},
        {TileId::IronBow, 1,   {{TileId::IronBar, 8},   {TileId::Gel, 3}}, Anvil},
        {TileId::GoldBow, 1,   {{TileId::GoldBar, 8},   {TileId::Gel, 3}}, Anvil},
        {TileId::Arrow, 10,    {{TileId::Stone, 2}, {TileId::Wood, 1}}, Workbench},
        {TileId::AncientSeed, 1, {{TileId::Wood, 15}, {TileId::Gel, 5}, {TileId::GoldBar, 1}}, Workbench},
    };
    return recipes;
}
