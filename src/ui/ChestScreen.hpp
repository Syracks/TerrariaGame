#pragma once

#include "items/ItemStack.hpp"
#include "world/World.hpp"
#include <array>
#include <raylib.h>

class Player;

constexpr int CHEST_GRID_W = 5;
constexpr int CHEST_GRID_H = 5;

class ChestScreen {
public:
    void open(int chestTX, int chestTY);
    void close();
    bool isOpen() const { return m_open; }
    int getChestX() const { return m_chestTX; }
    int getChestY() const { return m_chestTY; }

    void update(Player& player, World& world);
    void render(const Player& player, const World& world) const;

private:
    bool m_open = false;
    int m_chestTX = 0;
    int m_chestTY = 0;
    int m_hoveredSlot = -1;

    int getSlotAt(Vector2 mouse) const;
    Rectangle getPlayerSlotRect(int index) const;
    Rectangle getChestSlotRect(int index) const;
};
