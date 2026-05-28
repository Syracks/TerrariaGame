#pragma once

class Player;

class HUD {
public:
    static void render(const Player& player);
    static void renderPlayerHP(const Player& player);
};
