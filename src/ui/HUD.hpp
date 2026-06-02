#pragma once

#include <string>

class Player;
class Mob;

class HUD {
public:
    static void render(const Player& player, float dayTime);
    static void renderPlayerHP(const Player& player);
    static void renderTime(float dayTime);
    static void renderBossHP(const Mob& boss);

    static void update(float dt);
    static void showMessage(const std::string& text, float duration, int yOffset = 0);

private:
    struct Message {
        std::string text;
        float timer = 0.0f;
        float duration = 0.0f;
        int yOffset = 0;
    };
    static Message s_message;
    static constexpr float FADE_START = 1.5f;
};
