#include "../src/crafting/Recipe.hpp"
#include "../src/items/ItemStack.hpp"
#include "../src/items/ItemDefinition.hpp"
#include "../src/world/Tile.hpp"
#include "../src/world/TileRegistry.hpp"
#include <cassert>
#include <iostream>
#include <array>

void test_recipe_basic() {
    const auto& recipes = RecipeDatabase::getAll();
    assert(!recipes.empty());

    bool foundTorch = false;
    for (const auto& r : recipes) {
        if (r.result == TileId::Torch) {
            foundTorch = true;
            assert(r.resultCount == 3 || r.resultCount == 5);
            break;
        }
    }
    assert(foundTorch);

    std::cout << "test_recipe_basic: PASSED\n";
}

void test_available_recipes() {
    std::array<ItemStack, 5> slots = {{
        {TileId::Wood, 5},
        {TileId::Stone, 5},
        {TileId::Air, 0},
        {TileId::Air, 0},
        {TileId::Air, 0}
    }};

    auto available = RecipeDatabase::getAvailable(slots.begin(), slots.end(), CraftingStation::None);

    bool hasPlanks = false;
    for (const auto* r : available) {
        if (r->result == TileId::Planks) hasPlanks = true;
    }
    assert(hasPlanks);

    auto availableWorkbench = RecipeDatabase::getAvailable(slots.begin(), slots.end(), CraftingStation::Workbench);
    auto availableFurnace = RecipeDatabase::getAvailable(slots.begin(), slots.end(), CraftingStation::Furnace);

    std::cout << "test_available_recipes: PASSED\n";
}

void test_not_enough_ingredients() {
    std::array<ItemStack, 3> slots = {{
        {TileId::Wood, 1},
        {TileId::Air, 0},
        {TileId::Air, 0}
    }};

    auto available = RecipeDatabase::getAvailable(slots.begin(), slots.end(), CraftingStation::Workbench);
    bool hasWorkbench = false;
    for (const auto* r : available) {
        if (r->result == TileId::Workbench) hasWorkbench = true;
    }
    assert(!hasWorkbench);

    std::cout << "test_not_enough_ingredients: PASSED\n";
}

void test_station_filtering() {
    std::array<ItemStack, 10> slots = {{
        {TileId::Wood, 20},
        {TileId::Stone, 20},
        {TileId::CopperOre, 10},
        {TileId::Air, 0},
        {TileId::Air, 0},
        {TileId::Air, 0},
        {TileId::Air, 0},
        {TileId::Air, 0},
        {TileId::Air, 0},
        {TileId::Air, 0}
    }};

    auto hand = RecipeDatabase::getAvailable(slots.begin(), slots.end(), CraftingStation::None);
    auto furnace = RecipeDatabase::getAvailable(slots.begin(), slots.end(), CraftingStation::Furnace);

    bool hasCopperBar = false;
    for (const auto* r : furnace) {
        if (r->result == TileId::CopperBar) hasCopperBar = true;
    }
    assert(hasCopperBar);

    for (const auto* r : hand) {
        assert(r->station == CraftingStation::None);
    }

    std::cout << "test_station_filtering: PASSED\n";
}

int main() {
    TileRegistry::instance();
    ItemDatabase::instance();

    test_recipe_basic();
    test_available_recipes();
    test_not_enough_ingredients();
    test_station_filtering();

    std::cout << "\nAll recipe tests PASSED!\n";
    return 0;
}
