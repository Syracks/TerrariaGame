#include "../src/items/Inventory.hpp"
#include "../src/items/ItemDefinition.hpp"
#include "../src/items/ItemStack.hpp"
#include "../src/world/Tile.hpp"
#include "../src/world/TileRegistry.hpp"
#include "../src/core/Constants.hpp"
#include <cassert>
#include <iostream>

void test_add_remove() {
    Inventory inv;
    assert(inv.countItem(TileId::Dirt) == 0);

    assert(inv.addItem(TileId::Dirt, 10));
    assert(inv.countItem(TileId::Dirt) == 10);

    assert(inv.removeItem(TileId::Dirt, 3));
    assert(inv.countItem(TileId::Dirt) == 7);

    assert(!inv.removeItem(TileId::Dirt, 100));
    assert(inv.countItem(TileId::Dirt) == 7);

    assert(inv.removeItem(TileId::Dirt, 7));
    assert(inv.countItem(TileId::Dirt) == 0);

    std::cout << "test_add_remove: PASSED\n";
}

void test_max_stack() {
    Inventory inv;
    int maxStack = ItemDatabase::instance().getMaxStack(TileId::Dirt);
    assert(maxStack == 999);

    inv.addItem(TileId::Dirt, maxStack);
    assert(inv.countItem(TileId::Dirt) == maxStack);
    assert(inv.addItem(TileId::Dirt, 1));
    assert(inv.countItem(TileId::Dirt) == maxStack + 1);

    inv.removeItem(TileId::Dirt, maxStack + 1);

    int pickaxeStack = ItemDatabase::instance().getMaxStack(TileId::Pickaxe);
    assert(pickaxeStack == 1);

    inv.addItem(TileId::Pickaxe, 1);
    assert(inv.countItem(TileId::Pickaxe) == 1);
    inv.addItem(TileId::Pickaxe, 1);
    assert(inv.countItem(TileId::Pickaxe) == 2);

    inv.removeItem(TileId::Pickaxe, 2);

    std::cout << "test_max_stack_tool: PASSED\n";
}

void test_select_slot() {
    Inventory inv;
    assert(inv.getSelectedIndex() == 0);
    inv.selectSlot(2);
    assert(inv.getSelectedIndex() == 2);

    auto* slot = inv.getSelectedSlot();
    assert(slot != nullptr);
    assert(slot->tileId == TileId::Air);

    inv.selectSlot(99);
    assert(inv.getSelectedIndex() == 2);

    std::cout << "test_select_slot: PASSED\n";
}

int main() {
    TileRegistry::instance();
    ItemDatabase::instance();

    test_add_remove();
    test_max_stack();
    test_select_slot();

    std::cout << "\nAll inventory tests PASSED!\n";
    return 0;
}
