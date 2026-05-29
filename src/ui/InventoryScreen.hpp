#pragma once

#include "items/ItemStack.hpp"
#include <raylib.h>

class Player;

class InventoryScreen {
public:
    enum class Action { None, ReturnToMenu };

    Action update(Player& player);
    void render(const Player& player) const;

private:
    int m_scrollOffset = 0;
    ItemStack m_draggedStack;
    int m_hoveredSlot = -1;

    int getSlotAt(Vector2 mouse) const;
    Rectangle getSlotRect(int index) const;
};
