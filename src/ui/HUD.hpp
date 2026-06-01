#pragma once

#include <string>

class Player;

class HUD {
public:
    static void render(const Player& player, float dayTime);
    static void renderPlayerHP(const Player& player);
    static void renderTime(float dayTime);
};
