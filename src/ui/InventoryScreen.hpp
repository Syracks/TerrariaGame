#pragma once

#include "items/ItemStack.hpp"
#include "crafting/Recipe.hpp"
#include <raylib.h>
#include <vector>
#include <set>

class Player;
class World;

class InventoryScreen {
public:
    enum class Action { None, ReturnToMenu };

    Action update(Player& player, const World& world);
    void render(const Player& player, const World& world) const;

private:
    int m_scrollOffset = 0;
    ItemStack m_draggedStack;
    int m_hoveredSlot = -1;

    std::set<CraftingStation> detectStations(const Player& player, const World& world) const;

    int getSlotAt(Vector2 mouse) const;
    Rectangle getSlotRect(int index) const;
};
