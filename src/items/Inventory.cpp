#include "Inventory.hpp"
#include "world/TileRegistry.hpp"

Inventory::Inventory() {
    m_slots[0] = {TileId::Pickaxe, 1};
    m_slots[1] = {TileId::Axe, 1};
    m_slots[2] = {TileId::Sword, 1};
    for (int i = 3; i < constants::INVENTORY_SLOTS; ++i) {
        m_slots[i] = {TileId::Air, 0};
    }
}

bool Inventory::addItem(TileId tileId, int count) {
    if (tileId == TileId::Air) return false;

    for (auto& slot : m_slots) {
        if (slot.tileId == tileId) {
            slot.count += count;
            return true;
        }
    }

    for (auto& slot : m_slots) {
        if (slot.tileId == TileId::Air) {
            slot.tileId = tileId;
            slot.count = count;
            return true;
        }
    }
    return false;
}

bool Inventory::removeItem(TileId tileId, int count) {
    int available = countItem(tileId);
    if (available < count) return false;

    int remaining = count;
    for (auto& slot : m_slots) {
        if (slot.tileId == tileId && remaining > 0) {
            int taken = (slot.count < remaining) ? slot.count : remaining;
            slot.count -= taken;
            remaining -= taken;
            if (slot.count <= 0) {
                slot.tileId = TileId::Air;
                slot.count = 0;
            }
        }
    }
    return true;
}

int Inventory::countItem(TileId tileId) const {
    int total = 0;
    for (const auto& slot : m_slots) {
        if (slot.tileId == tileId) {
            total += slot.count;
        }
    }
    return total;
}

ItemStack* Inventory::getSelectedSlot() {
    return &m_slots[m_selectedSlot];
}

const ItemStack* Inventory::getSelectedSlot() const {
    return &m_slots[m_selectedSlot];
}

void Inventory::selectSlot(int index) {
    if (index >= 0 && index < constants::HOTBAR_SLOTS) {
        m_selectedSlot = index;
    }
}
