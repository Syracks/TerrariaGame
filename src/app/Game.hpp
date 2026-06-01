#pragma once

#include "GameState.hpp"
#include "CameraController.hpp"
#include "PlayerController.hpp"
#include "ui/MainMenu.hpp"
#include "ui/SettingsMenu.hpp"
#include "ui/InventoryScreen.hpp"
#include "ui/ChestScreen.hpp"
#include "ui/HUD.hpp"
#include "GameSession.hpp"
#include "GameRenderer.hpp"
#include <memory>

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void init();
    void handleInput();
    void update(float dt);
    void cleanup();
    void newGame(const std::string& name, WorldSize size, int slot);
    void loadGame(int slot);
    void saveGame();
    void cleanupWorld();

    GameState m_state;
    CameraController m_camera;
    PlayerController m_playerController;
    InventoryScreen m_inventoryScreen;
    ChestScreen m_chestScreen;
    MainMenu m_menu;
    SettingsMenu m_settingsMenu;
    GameRenderer m_renderer;

    GameSession m_session;

    float m_autosaveTimer = 0.0f;
    int m_spawnGuard = 0;
    int m_totalKills = 0;
};
