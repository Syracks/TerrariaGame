#include "Game.hpp"
#include "core/Constants.hpp"
#include "core/Input.hpp"
#include "core/Math.hpp"
#include "core/TextureManager.hpp"
#include "core/SoundManager.hpp"
#include "items/ItemDefinition.hpp"
#include "items/Tool.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/LiquidSystem.hpp"
#include "systems/MiningSystem.hpp"
#include "ui/HUD.hpp"
#include "ui/Minimap.hpp"
#include "save/SaveManager.hpp"
#include "world/TileRegistry.hpp"
#include "world/WorldGenerator.hpp"
#include "app/MobSpawner.hpp"
#include "systems/CombatSystem.hpp"
#include "systems/InteractionSystem.hpp"
#include "systems/DeathSystem.hpp"

#include <raylib.h>
#include <ctime>
#include <fstream>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace {
    constexpr float AUTOSAVE_INTERVAL = 60.0f;
}

Game::Game()
    : m_state(GameState::MainMenu) {
    ItemDatabase::instance();
}

Game::~Game() = default;

void Game::init() {
    InitWindow(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, "Terraria");
    SetTargetFPS(constants::TARGET_FPS);
    SetExitKey(0);

    InitAudioDevice();
    SoundManager::instance().loadAll();

    m_renderer.init();

    TextureManager::instance().loadAll();

    m_state = GameState::MainMenu;
}

void Game::run() {
    init();

    while (!WindowShouldClose() && m_state != GameState::Quit) {
        float dt = GetFrameTime();

        handleInput();

        if (m_state == GameState::Playing) {
            update(dt);
        }

        BeginDrawing();

        if (m_state == GameState::MainMenu) {
            m_renderer.renderMainMenu(m_menu);
        } else if (m_state == GameState::Settings) {
            m_renderer.renderSettingsMenu(m_settingsMenu);
        } else if (m_state == GameState::Playing || m_state == GameState::Inventory || m_state == GameState::Chest) {
            m_renderer.renderGame(m_session, m_camera.getCamera(),
                                  m_state == GameState::Inventory, m_inventoryScreen,
                                  m_session.getWorld());
            if (m_state == GameState::Chest && m_chestScreen.isOpen()) {
                m_chestScreen.render(m_session.getPlayer(), m_session.getWorld());
            }
        }

        EndDrawing();
    }

    cleanup();
}

void Game::handleInput() {
    if (m_state == GameState::MainMenu) {
        auto result = m_menu.update();
        switch (result.type) {
            case MenuResult::StartNewGame:
                newGame(result.worldName, result.worldSize, result.slot);
                m_state = GameState::Playing;
                m_menu.setVisible(false);
                break;
            case MenuResult::LoadSlot:
                loadGame(result.slot);
                m_state = GameState::Playing;
                m_menu.setVisible(false);
                break;
            case MenuResult::Settings:
                m_state = GameState::Settings;
                break;
            case MenuResult::Quit:
                m_state = GameState::Quit;
                break;
            default:
                break;
        }
        return;
    }

    if (m_state == GameState::Settings) {
        auto action = m_settingsMenu.update();
        if (action == SettingsMenu::Action::Back) {
            m_state = GameState::MainMenu;
        }
        return;
    }

    if (input::isPausePressed()) {
        if (m_state == GameState::Playing) {
            m_state = GameState::Inventory;
        } else if (m_state == GameState::Inventory) {
            m_state = GameState::Playing;
        } else if (m_state == GameState::Chest) {
            m_chestScreen.close();
            m_state = GameState::Playing;
        }
        return;
    }

    if (m_state == GameState::Inventory) {
        auto action = m_inventoryScreen.update(m_session.getPlayer(), m_session.getWorld());
        if (action == InventoryScreen::Action::ReturnToMenu) {
            cleanupWorld();
            m_state = GameState::MainMenu;
            m_menu.setVisible(true);
        }
        return;
    }

    if (m_state == GameState::Chest) {
        m_chestScreen.update(m_session.getPlayer(), m_session.getWorld());
        return;
    }

    if (m_state != GameState::Playing)
        return;

    if (input::isMinimapToggled()) {
        m_session.toggleMinimap();
    }

    int slot = input::getHotbarSelection();
    if (slot >= 0) m_session.getPlayer().getInventory().selectSlot(slot);

    int wheel = GetMouseWheelMove();
    if (wheel != 0) {
        int current = m_session.getPlayer().getInventory().getSelectedIndex();
        current -= wheel;
        if (current < 0) current += constants::HOTBAR_SLOTS;
        if (current >= constants::HOTBAR_SLOTS) current %= constants::HOTBAR_SLOTS;
        m_session.getPlayer().getInventory().selectSlot(current);
    }

    if (input::isSavePressed()) saveGame();

    if (input::isMinePressed()) {
        InteractionSystem::handleMinePress(m_session.getPlayer(), m_session.getWorld(),
                                            m_camera.getCamera());
    }

    if (input::isPlacePressed()) {
        auto& world = m_session.getWorld();
        auto& player = m_session.getPlayer();
        Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), m_camera.getCamera());
        int tx = math::worldToTileX(worldPos.x);
        int ty = math::worldToTileY(worldPos.y);
        if (world.isInBounds(tx, ty) && world.getTile(tx, ty) == TileId::ChestBlock) {
            float distX = std::abs(player.getPosition().x + player.getBounds().width / 2 -
                                   (math::tileToWorldX(tx) + constants::TILE_SIZE / 2.0f));
            float distY = std::abs(player.getPosition().y + player.getBounds().height / 2 -
                                   (math::tileToWorldY(ty) + constants::TILE_SIZE / 2.0f));
            if (distX <= 3.0f * constants::TILE_SIZE && distY <= 3.0f * constants::TILE_SIZE) {
                m_chestScreen.open(tx, ty);
                SoundManager::instance().play(SoundManager::BlockPlace);
                m_state = GameState::Chest;
                return;
            }
        }
        InteractionSystem::handlePlacePress(m_session.getPlayer(), m_session.getWorld(),
                                             m_camera.getCamera(), m_session.getMinimap());
    }
}

void Game::update(float dt) {
    auto& world = m_session.getWorld();
    auto& player = m_session.getPlayer();
    auto& mobs = m_session.getMobs();

    if (dt > 0.033f) dt = 0.033f;

    m_session.advanceDayTime(dt);

    static int spawnGuard = 10;
    if (spawnGuard > 0) {
        Vector2 safe = DeathSystem::findSafeSpawnPosition(world);
        player.setPosition(safe);
        player.setVelocity({0, 0});
        spawnGuard--;
    }

    if (player.getHealth() > 0) {
        bool inWater = LiquidSystem::isInWater(world, player.getPosition().x, player.getPosition().y,
                                                player.getBounds().width, player.getBounds().height);

        if (!player.isOnGround() && input::isJumpPressed() && inWater) {
            Vector2 vel = player.getVelocity();
            vel.y = -200.0f;
            player.setVelocity(vel);
        }

        m_playerController.update(player);
        player.update(dt);
        CollisionSystem::resolveCollision(player, world, dt);

        if (inWater) {
            Vector2 vel = player.getVelocity();
            vel.x *= 0.85f;
            vel.y *= 0.92f;
            if (std::abs(vel.y) > 60.0f) vel.y *= 0.96f;
            player.setVelocity(vel);
        }
    }

    LiquidSystem::update(world, dt);

    if (LiquidSystem::isInLava(world, player.getPosition().x, player.getPosition().y,
                                player.getBounds().width, player.getBounds().height)) {
        player.takeDamage(10);
    }

    Vector2 playerCenter = {
        player.getPosition().x + player.getBounds().width / 2,
        player.getPosition().y + player.getBounds().height / 2
    };

    for (auto& mob : mobs) {
        mob->setPlayerPos(playerCenter);
        mob->update(dt);
        PhysicsSystem::update(*mob, world, dt);
    }

    m_camera.update(player);

    if (m_session.getDeathTimer() > 0.0f) {
        player.setVelocity({0, 0});
    }

    DeathSystem::updateDeath(player, world, m_session.getMobs(),
                              m_session.getDeathTimerRef(), dt,
                              m_session.getParticles(),
                              m_session.getRNG());

    if (player.getHealth() > 0) {
        InteractionSystem::handleSwingCompletion(player, world,
                                                  m_session.getParticles(),
                                                  m_session.getMinimap());

        CombatSystem::checkSwordHit(player, mobs, m_session.getParticles());
        CombatSystem::checkMobContactDamage(mobs, player);

        for (auto it = mobs.begin(); it != mobs.end(); ) {
            if ((*it)->getHealth() <= 0) {
                auto& mob = *it;
                SoundManager::instance().play(SoundManager::MobDeath);
                for (int i = 0; i < 8; ++i) {
                    float px = mob->getBounds().x + mob->getBounds().width / 2;
                    float py = mob->getBounds().y + mob->getBounds().height / 2;
                    m_session.getParticles().emit({px, py}, {0, -200}, {150, 150, 150, 255}, 0.6f, 4, 1);
                }

                auto& rng = m_session.getRNG();
                std::uniform_int_distribution<int> dist100(0, 99);
                std::uniform_int_distribution<int> distCount(1, 3);
                TileId dropItem = TileId::Air;
                int dropCount = 0;
                if (mob->getType() == MobType::Slime || mob->getType() == MobType::BlueSlime) {
                    if (dist100(rng) < 70) {
                        dropItem = TileId::Gel;
                        dropCount = distCount(rng);
                    }
                } else if (mob->getType() == MobType::Zombie) {
                    if (dist100(rng) < 30) {
                        dropItem = TileId::CopperOre;
                        dropCount = 1;
                    }
                }
                if (dropItem != TileId::Air) {
                    player.getInventory().addItem(dropItem, dropCount);
                }

                m_totalKills++;
                mob->unload();
                it = mobs.erase(it);
            } else {
                ++it;
            }
        }
    }

    m_session.getParticles().update(dt);

    MobSpawner::updateNightSpawning(world, mobs, player, m_session.getDayTime(), dt, m_session.getRNG());

    m_autosaveTimer += dt;
    if (m_autosaveTimer >= AUTOSAVE_INTERVAL) {
        saveGame();
        m_autosaveTimer = 0.0f;
    }
}

void Game::newGame(const std::string& name, WorldSize size, int slot) {
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
    m_session.newWorld(name, size, slot, seed);

    auto& world = m_session.getWorld();
    auto& player = m_session.getPlayer();
    auto& rng = m_session.getRNG();

    BeginDrawing();
    ClearBackground(Color{20, 20, 30, 255});
    const char* loadingTitle = "Generating World...";
    int loadingSize = 40;
    int loadingW = MeasureText(loadingTitle, loadingSize);
    DrawText(loadingTitle, (constants::SCREEN_WIDTH - loadingW) / 2,
             constants::SCREEN_HEIGHT / 2 - 80, loadingSize, GREEN);
    int barW = 400, barH = 28;
    int barX = (constants::SCREEN_WIDTH - barW) / 2;
    int barY = constants::SCREEN_HEIGHT / 2 - 10;
    DrawRectangle(barX, barY, barW, barH, Color{40, 40, 50, 255});
    DrawRectangleLines(barX, barY, barW, barH, Color{80, 80, 120, 255});
    DrawText("Preparing...", (constants::SCREEN_WIDTH - MeasureText("Preparing...", 20)) / 2,
             barY + barH + 12, 20, Color{120, 120, 140, 255});
    EndDrawing();

    world.generate(seed, [&](float progress) {
        BeginDrawing();
        ClearBackground(Color{20, 20, 30, 255});
        DrawText(loadingTitle, (constants::SCREEN_WIDTH - loadingW) / 2,
                 constants::SCREEN_HEIGHT / 2 - 80, loadingSize, GREEN);
        DrawRectangle(barX, barY, barW, barH, Color{40, 40, 50, 255});
        DrawRectangleLines(barX, barY, barW, barH, Color{80, 80, 120, 255});
        int fillW = static_cast<int>((barW - 4) * progress);
        if (fillW > 0) DrawRectangle(barX + 2, barY + 2, fillW, barH - 4, GREEN);
        const char* phases[] = {"Terrain...", "Caves...", "Cabins...", "Ores...", "Pockets...", "Trees...", "Islands..."};
        int phaseIdx = std::min(static_cast<int>(progress * 7), 6);
        DrawText(phases[phaseIdx], (constants::SCREEN_WIDTH - MeasureText(phases[phaseIdx], 20)) / 2,
                 barY + barH + 12, 20, Color{120, 120, 140, 255});
        std::string pct = std::to_string(static_cast<int>(progress * 100)) + "%";
        DrawText(pct.c_str(), barX + barW + 10, barY + 4, 20, GREEN);
        EndDrawing();
    });

    auto give = [&](TileId id, int count = 1) { player.getInventory().addItem(id, count); };
    give(TileId::CopperPickaxe, 1);
    give(TileId::CopperAxe, 1);
    give(TileId::CopperSword, 1);

    m_session.getMinimap().rebuild(world);

    MobSpawner::spawnSlimes(world, m_session.getMobs(), player, rng);

    Vector2 spawnPos = DeathSystem::findSafeSpawnPosition(world);
    player.setPosition(spawnPos);
    player.setVelocity({0, 0});

    m_camera.update(player);

    saveGame();
    m_totalKills = 0;
    m_autosaveTimer = 0.0f;
}

void Game::loadGame(int slot) {
    std::string dataPath = SaveManager::getSlotDataPath(slot);
    if (!SaveManager::saveExists(dataPath)) return;

    SlotInfo info = SaveManager::getSlotInfo(slot);
    WorldDimensions dims = getWorldDimensions(info.size);
    constants::WORLD_WIDTH = dims.width;
    constants::WORLD_HEIGHT = dims.height;

    auto world = std::make_unique<World>();
    auto player = std::make_unique<Player>();
    player->load();

    SaveManager::loadSlot(slot, *world, *player);

    {
        auto pos = player->getPosition();
        std::ofstream log("/tmp/opencode_spawn_debug.log", std::ios::app);
        log << "LOAD: player pos=(" << pos.x << "," << pos.y << ")\n";
        log << "world size=" << constants::WORLD_WIDTH << "x" << constants::WORLD_HEIGHT << "\n";
        log.close();
    }

    m_session.adoptWorld(std::move(world), std::move(player), slot, info.size, info.name, info.seed);

    {
        auto& p = m_session.getPlayer();
        Vector2 spawnPos = DeathSystem::findSafeSpawnPosition(m_session.getWorld());
        p.setPosition(spawnPos);
        p.setVelocity({0, 0});
        auto pos = p.getPosition();
        std::ofstream log("/tmp/opencode_spawn_debug.log", std::ios::app);
        log << "LOAD-SPAWN: player pos=(" << pos.x << "," << pos.y << ")\n";
        log.close();
    }

    m_session.getMinimap().rebuild(m_session.getWorld());

    m_camera.update(m_session.getPlayer());
    {
        auto& p = m_session.getPlayer();
        auto pos = p.getPosition();
        std::ofstream log("/tmp/opencode_spawn_debug.log", std::ios::app);
        log << "AFTER ADOPT: player pos=(" << pos.x << "," << pos.y << ")\n";
        log.close();
    }
}

void Game::saveGame() {
    int slot = m_session.getCurrentSlot();
    if (slot < 0) return;
    SaveManager::saveSlot(slot, m_session.getWorld(), m_session.getPlayer(),
                          m_session.getWorldName(), m_session.getWorldSize(), m_session.getSeed());
    std::cout << "Game saved to slot " << slot << "." << std::endl;
}

void Game::cleanupWorld() {
    m_session.clear();
}

void Game::cleanup() {
    m_session.clear();
    m_renderer.cleanup();
    TextureManager::instance().unloadAll();
    SoundManager::instance().unloadAll();
    CloseAudioDevice();
    CloseWindow();
}
