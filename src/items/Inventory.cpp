#include "Inventory.hpp"
#include "ItemDefinition.hpp"

Inventory::Inventory() {
    clear();
}

void Inventory::clear() {
    for (auto& slot : m_slots) {
        slot = {TileId::Air, 0};
    }
}

bool Inventory::addItem(TileId tileId, int count) {
    if (tileId == TileId::Air || count <= 0) return false;

    int maxStack = ItemDatabase::instance().getMaxStack(tileId);
    if (maxStack <= 0) return false;

    int remaining = count;

    for (auto& slot : m_slots) {
        if (slot.tileId == tileId && slot.count < maxStack) {
            int space = maxStack - slot.count;
            int add = (remaining < space) ? remaining : space;
            slot.count += add;
            remaining -= add;
            if (remaining <= 0) return true;
        }
    }

    for (auto& slot : m_slots) {
        if (slot.tileId == TileId::Air) {
            int add = (remaining < maxStack) ? remaining : maxStack;
            slot.tileId = tileId;
            slot.count = add;
            remaining -= add;
            if (remaining <= 0) return true;
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
