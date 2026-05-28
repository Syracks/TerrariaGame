#pragma once

#include "ItemStack.hpp"
#include "core/Constants.hpp"
#include <array>

class Inventory {
public:
    Inventory();

    bool addItem(TileId tileId, int count);
    bool removeItem(TileId tileId, int count);
    int countItem(TileId tileId) const;
    ItemStack* getSelectedSlot();
    const ItemStack* getSelectedSlot() const;
    void selectSlot(int index);
    int getSelectedIndex() const { return m_selectedSlot; }

    std::array<ItemStack, constants::INVENTORY_SLOTS>& getSlots() { return m_slots; }
    const std::array<ItemStack, constants::INVENTORY_SLOTS>& getSlots() const { return m_slots; }

private:
    std::array<ItemStack, constants::INVENTORY_SLOTS> m_slots;
    int m_selectedSlot = 0;
};
