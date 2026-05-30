#pragma once

#include <raylib.h>
#include "core/Constants.hpp"
#include "ui/HUD.hpp"
#include <vector>

class GameSession;
class Player;
class World;
class InventoryScreen;
class MainMenu;
class SettingsMenu;

class GameRenderer {
public:
    GameRenderer();

    void init();
    void cleanup();

    void renderMainMenu(MainMenu& menu);
    void renderSettingsMenu(SettingsMenu& menu);
    void renderGame(GameSession& session, const Camera2D& camera,
                    bool inventoryOpen, const InventoryScreen& inventoryScreen,
                    const World& world);

private:
    void renderSky(float dayTime);
    void renderBackground(float dayTime, const World& world, const Player& player);
    void renderWorldAndEntities(GameSession& session, const Camera2D& camera);
    void renderHUD(GameSession& session);
    void renderMinimap(GameSession& session);
    void renderDeathOverlay(float deathTimer);

    Texture2D loadBackground(const char* path);
    void unloadBackground(Texture2D& tex);

    Texture2D m_forestBg{};
    Texture2D m_desertBg{};
    Texture2D m_snowBg{};
    Texture2D m_jungleBg{};
    Texture2D m_undergroundBg{};
    Texture2D m_beachBg{};
};
