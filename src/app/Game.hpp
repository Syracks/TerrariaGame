#pragma once

#include "GameState.hpp"
#include "CameraController.hpp"
#include "PlayerController.hpp"
#include "world/World.hpp"
#include "entities/Player.hpp"
#include "entities/Mob.hpp"
#include "ui/MainMenu.hpp"
#include "ui/SettingsMenu.hpp"
#include "ui/InventoryScreen.hpp"
#include "ui/Minimap.hpp"
#include "systems/ParticleSystem.hpp"
#include "core/Constants.hpp"
#include "core/SoundManager.hpp"
#include <memory>
#include <vector>
#include <string>

class Game {
public:
    Game();
    ~Game();

    void run();

private:
    void init();
    void update(float dt);
    void render();
    void cleanup();
    void handleInput();
    void newGame(const std::string& name, WorldSize size, int slot);
    void loadGame(int slot);
    void saveGame();
    void cleanupWorld();
    void spawnSlimes();
    void spawnZombies();

    GameState m_state;
    std::unique_ptr<World> m_world;
    std::unique_ptr<Player> m_player;
    CameraController m_camera;
    PlayerController m_playerController;
    InventoryScreen m_inventoryScreen;
    MainMenu m_menu;
    SettingsMenu m_settingsMenu;
    Texture2D m_background{};

    std::vector<std::unique_ptr<Mob>> m_mobs;
    ParticleSystem m_particles;

    std::unique_ptr<Minimap> m_minimap;
    bool m_minimapVisible = true;

    float m_deathTimer = 0.0f;
    float m_dayTime = 0.0f;
    unsigned int m_seed;
    int m_currentSlot = -1;
    WorldSize m_worldSize = WorldSize::Medium;
    std::string m_worldName;
};
