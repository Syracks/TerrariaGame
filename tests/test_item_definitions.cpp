#include "../src/items/ItemDefinition.hpp"
#include "../src/world/Tile.hpp"
#include "../src/world/TileRegistry.hpp"
#include <cassert>
#include <iostream>

void test_item_properties() {
    auto& db = ItemDatabase::instance();

    const auto& dirt = db.get(TileId::Dirt);
    assert(dirt.name == "Dirt");
    assert(dirt.maxStack == 999);
    assert(dirt.placeable == true);
    assert(dirt.consumable == false);

    const auto& pickaxe = db.get(TileId::Pickaxe);
    assert(pickaxe.name == "Wood Pickaxe");
    assert(pickaxe.maxStack == 1);
    assert(pickaxe.placeable == false);
    assert(pickaxe.tool.miningLevel == 0);
    assert(pickaxe.tool.damage == 4);

    const auto& copperPick = db.get(TileId::CopperPickaxe);
    assert(copperPick.name == "Copper Pickaxe");
    assert(copperPick.tool.miningLevel == 1);
    assert(copperPick.tool.miningSpeed > 1.0f);

    const auto& torch = db.get(TileId::Torch);
    assert(torch.maxStack == 99);

    const auto& workbench = db.get(TileId::Workbench);
    assert(workbench.maxStack == 1);
    assert(workbench.placeable == true);

    std::cout << "test_item_properties: PASSED\n";
}

void test_max_stack() {
    assert(ItemDatabase::instance().getMaxStack(TileId::Dirt) == 999);
    assert(ItemDatabase::instance().getMaxStack(TileId::Pickaxe) == 1);
    assert(ItemDatabase::instance().getMaxStack(TileId::Torch) == 99);
    assert(ItemDatabase::instance().getMaxStack(TileId::Workbench) == 1);
    assert(ItemDatabase::instance().getMaxStack(TileId::Water) == 999);

    std::cout << "test_max_stack: PASSED\n";
}

void test_tool_stats() {
    auto& db = ItemDatabase::instance();

    const auto& woodAxe = db.get(TileId::Axe);
    assert(woodAxe.tool.damage == 4);

    const auto& goldSword = db.get(TileId::GoldSword);
    assert(goldSword.tool.damage == 30);
    assert(goldSword.tool.miningLevel == 3);

    const auto& ironPick = db.get(TileId::IronPickaxe);
    assert(ironPick.tool.miningLevel == 2);
    assert(ironPick.tool.miningSpeed > 1.0f);

    std::cout << "test_tool_stats: PASSED\n";
}

int main() {
    TileRegistry::instance();
    ItemDatabase::instance();

    test_item_properties();
    test_max_stack();
    test_tool_stats();

    std::cout << "\nAll item definition tests PASSED!\n";
    return 0;
}
