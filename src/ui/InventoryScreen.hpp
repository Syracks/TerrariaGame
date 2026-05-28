#pragma once

class Player;

class InventoryScreen {
public:
    enum class Action { None, ReturnToMenu };

    Action update(Player& player);
    void render(const Player& player) const;

private:
    int m_scrollOffset = 0;
};
