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
    InitWindow(constants::VIRTUAL_WIDTH, constants::VIRTUAL_HEIGHT, "Terraria");
    SetTargetFPS(constants::TARGET_FPS);
    SetExitKey(0);

    m_target = LoadRenderTexture(constants::VIRTUAL_WIDTH, constants::VIRTUAL_HEIGHT);
    SetTextureFilter(m_target.texture, TEXTURE_FILTER_POINT);

    InitAudioDevice();
    SoundManager::instance().loadAll();
    applySettings();

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

        BeginTextureMode(m_target);
        ClearBackground(Color{20, 20, 30, 255});

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

        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);

        Rectangle dest = math::getScaledDestRect();
        DrawTexturePro(
            m_target.texture,
            { 0.0f, 0.0f,
              static_cast<float>(m_target.texture.width),
              static_cast<float>(-m_target.texture.height) },
            dest,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );

        EndDrawing();
    }

    cleanup();
}

void Game::toggleFullscreen() {
    int monitor = GetCurrentMonitor();

    if (!IsWindowFullscreen()) {
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else {
        ToggleFullscreen();
        int idx = m_settingsMenu.getResolutionIndex();
        SetWindowSize(constants::RESOLUTIONS[idx][0], constants::RESOLUTIONS[idx][1]);
    }
    m_settingsMenu.setFullscreen(IsWindowFullscreen());
}

void Game::applySettings() {
    SoundManager::instance().setMasterVolume(m_settingsMenu.getVolume());

    if (m_settingsMenu.getStoredFullscreen() && !IsWindowFullscreen()) {
        int monitor = GetCurrentMonitor();
        SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
        ToggleFullscreen();
    } else if (!m_settingsMenu.getStoredFullscreen()) {
        if (IsWindowFullscreen()) {
            ToggleFullscreen();
        }
        SetWindowSize(
            constants::RESOLUTIONS[m_settingsMenu.getResolutionIndex()][0],
            constants::RESOLUTIONS[m_settingsMenu.getResolutionIndex()][1]
        );
    }

    m_session.setMinimapVisible(m_settingsMenu.getShowMinimap());
}

void Game::handleInput() {
    if (IsKeyPressed(KEY_F11)) {
        toggleFullscreen();
    }

    if (m_state == GameState::MainMenu) {
        auto result = m_menu.update();
        switch (result.type) {
            case MenuResult::StartNewGame:
                newGame(result.worldName, result.worldSize, result.slot, result.difficulty);
                m_state = GameState::Playing;
                m_menu.setVisible(false);
                break;
            case MenuResult::LoadSlot:
                if (loadGame(result.slot)) {
                    m_state = GameState::Playing;
                    m_menu.setVisible(false);
                }
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
        Vector2 mouse = math::getVirtualMouse();
        Vector2 worldPos = GetScreenToWorld2D(mouse, m_camera.getCamera());
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

    if (m_spawnGuard > 0) {
        Vector2 safe = DeathSystem::findSafeSpawnPosition(world);
        player.setPosition(safe);
        player.setVelocity({0, 0});
        m_spawnGuard--;
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

    m_liquidSystem.update(world, dt);

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

    bool respawned = DeathSystem::updateDeath(player, world, m_session.getMobs(),
                              m_session.getDeathTimerRef(), dt,
                              m_session.getParticles(),
                              m_session.getRNG());

    if (respawned && m_session.getDifficulty() == Difficulty::Hardcore) {
        SaveManager::deleteSlot(m_session.getCurrentSlot());
        cleanupWorld();
        m_state = GameState::MainMenu;
        m_menu.setVisible(true);
        return;
    }

    if (player.getHealth() > 0) {
        InteractionSystem::handleSwingCompletion(player, world,
                                                   m_session.getParticles(),
                                                   m_session.getMinimap());

        if (player.isBowFired()) {
            player.resetBowFired();
            auto* sel = player.getInventory().getSelectedSlot();
            if (sel) {
                Arrow arrow;
                float centerX = player.getPosition().x + player.getBounds().width / 2;
                float centerY = player.getPosition().y + player.getBounds().height / 2;
                arrow.position = {centerX, centerY - 4};
                Vector2 mouse = math::getVirtualMouse();
                Vector2 worldPos = GetScreenToWorld2D(mouse, m_camera.getCamera());
                float dx = worldPos.x - centerX;
                float dy = worldPos.y - centerY;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.1f) {
                    float arrowSpeed = 900.0f;
                    arrow.velocity = {dx / len * arrowSpeed, dy / len * arrowSpeed};
                    arrow.facing = dx < 0 ? -1 : 1;
                } else {
                    int facing = player.isFacingLeft() ? -1 : 1;
                    arrow.velocity = {static_cast<float>(facing) * 900.0f, -50.0f};
                    arrow.facing = facing;
                }
                m_session.getArrows().push_back(arrow);
                SoundManager::instance().play(SoundManager::SwordHit);
            }
        }

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

    {
        auto& arrows = m_session.getArrows();
        auto& particles = m_session.getParticles();
        const float GRAVITY = 800.0f;
        for (auto it = arrows.begin(); it != arrows.end(); ) {
            Arrow& a = *it;
            a.lifetime -= dt;
            if (!a.active || a.lifetime <= 0.0f) {
                it = arrows.erase(it);
                continue;
            }
            a.velocity.y += GRAVITY * dt;
            a.position.x += a.velocity.x * dt;
            a.position.y += a.velocity.y * dt;

            int tx = math::worldToTileX(a.position.x);
            int ty = math::worldToTileY(a.position.y);
            bool hitTile = false;
            if (world.isInBounds(tx, ty)) {
                TileId tile = world.getTile(tx, ty);
                if (tile != TileId::Air && TileRegistry::instance().get(tile).solid) {
                    hitTile = true;
                }
            }
            if (a.position.x < 0 || a.position.x > constants::WORLD_WIDTH * constants::TILE_SIZE ||
                a.position.y < 0 || a.position.y > constants::WORLD_HEIGHT * constants::TILE_SIZE) {
                hitTile = true;
            }

            bool hitMob = false;
            for (auto& mob : mobs) {
                Rectangle mobBounds = mob->getBounds();
                if (CheckCollisionPointRec(a.position, mobBounds)) {
                    int damage = 0;
                    auto* sel = player.getInventory().getSelectedSlot();
                    if (sel) damage = ItemDatabase::instance().get(sel->tileId).tool.damage;
                    if (damage <= 0) damage = 10;
                    mob->takeDamage(damage);
                    for (int i = 0; i < 5; ++i) {
                        particles.emit(a.position, {0, -100}, {255, 100, 50, 255}, 0.4f, 3, 1);
                    }
                    hitMob = true;
                    break;
                }
            }

            if (hitTile || hitMob) {
                for (int i = 0; i < 4; ++i) {
                    particles.emit(a.position,
                        {a.velocity.x * 0.2f, a.velocity.y * 0.2f},
                        {200, 180, 140, 255}, 0.5f, 3, 1);
                }
                it = arrows.erase(it);
            } else {
                ++it;
            }
        }
    }

    m_session.getParticles().update(dt);

    Difficulty diff = m_session.getDifficulty();
    m_mobSpawner.updateNightSpawning(world, mobs, player, m_session.getDayTime(), dt, m_session.getRNG(), diff);

    m_autosaveTimer += dt;
    if (m_autosaveTimer >= AUTOSAVE_INTERVAL) {
        saveGame();
        m_autosaveTimer = 0.0f;
    }
}

void Game::newGame(const std::string& name, WorldSize size, int slot, Difficulty difficulty) {
    unsigned int seed = static_cast<unsigned int>(std::time(nullptr));
    m_session.newWorld(name, size, slot, seed, difficulty);

    auto& world = m_session.getWorld();
    auto& player = m_session.getPlayer();
    auto& rng = m_session.getRNG();

    const char* loadingTitle = "Generating World...";
    int loadingSize = 40;
    int loadingW = MeasureText(loadingTitle, loadingSize);
    int barW = 400, barH = 28;
    int barX = (constants::VIRTUAL_WIDTH - barW) / 2;
    int barY = constants::VIRTUAL_HEIGHT / 2 - 10;

    auto renderLoadingFrame = [&](const char* phase, float progress) {
        BeginTextureMode(m_target);
        ClearBackground(BLACK);
        DrawText(loadingTitle, (constants::VIRTUAL_WIDTH - loadingW) / 2,
                 constants::VIRTUAL_HEIGHT / 2 - 80, loadingSize, GREEN);
        DrawRectangle(barX, barY, barW, barH, Color{40, 40, 50, 255});
        DrawRectangleLines(barX, barY, barW, barH, Color{80, 80, 120, 255});
        if (progress > 0.0f) {
            int fillW = static_cast<int>((barW - 4) * progress);
            if (fillW > 0) DrawRectangle(barX + 2, barY + 2, fillW, barH - 4, GREEN);
            int phaseW = MeasureText(phase, 20);
            DrawText(phase, (constants::VIRTUAL_WIDTH - phaseW) / 2,
                     barY + barH + 12, 20, Color{120, 120, 140, 255});
            std::string pct = std::to_string(static_cast<int>(progress * 100)) + "%";
            DrawText(pct.c_str(), barX + barW + 10, barY + 4, 20, GREEN);
        } else {
            DrawText("Preparing...", (constants::VIRTUAL_WIDTH - MeasureText("Preparing...", 20)) / 2,
                     barY + barH + 12, 20, Color{120, 120, 140, 255});
        }
        EndTextureMode();

        BeginDrawing();
        ClearBackground(BLACK);
        Rectangle dest = math::getScaledDestRect();
        DrawTexturePro(
            m_target.texture,
            { 0.0f, 0.0f,
              static_cast<float>(m_target.texture.width),
              static_cast<float>(-m_target.texture.height) },
            dest,
            { 0.0f, 0.0f },
            0.0f,
            WHITE
        );
        EndDrawing();
    };

    renderLoadingFrame("Preparing...", 0.0f);

    world.generate(seed, [&](float progress) {
        const char* phases[] = {"Terrain...", "Caves...", "Cabins...", "Ores...", "Pockets...", "Trees...", "Islands..."};
        int phaseIdx = std::min(static_cast<int>(progress * 7), 6);
        renderLoadingFrame(phases[phaseIdx], progress);
    });

    auto give = [&](TileId id, int count = 1) { player.getInventory().addItem(id, count); };
    give(TileId::CopperPickaxe, 1);
    give(TileId::CopperAxe, 1);
    give(TileId::CopperSword, 1);
    give(TileId::CopperBow, 1);
    give(TileId::IronBow, 1);
    give(TileId::GoldBow, 1);
    give(TileId::Arrow, 99);

    m_session.getMinimap().rebuild(world);

    m_mobSpawner.spawnSlimes(world, m_session.getMobs(), player, rng, difficulty);

    Vector2 spawnPos = DeathSystem::findSafeSpawnPosition(world);
    player.setPosition(spawnPos);
    player.setVelocity({0, 0});

    m_session.setMinimapVisible(m_settingsMenu.getShowMinimap());

    m_camera.update(player);

    m_mobSpawner.reset();
    saveGame();
    m_totalKills = 0;
    m_autosaveTimer = 0.0f;
    m_spawnGuard = 10;
}

bool Game::loadGame(int slot) {
    std::string dataPath = SaveManager::getSlotDataPath(slot);
    if (!SaveManager::saveExists(dataPath)) {
        std::cerr << "No save data found for slot " << slot << std::endl;
        return false;
    }

    SlotInfo info = SaveManager::getSlotInfo(slot);
    WorldDimensions dims = getWorldDimensions(info.size);
    constants::WORLD_WIDTH = dims.width;
    constants::WORLD_HEIGHT = dims.height;

    auto world = std::make_unique<World>();
    auto player = std::make_unique<Player>();
    player->load();

    float dayTime = 0.0f;
    if (!SaveManager::loadSlot(slot, *world, *player, dayTime)) {
        std::cerr << "Failed to load save data for slot " << slot << std::endl;
        return false;
    }

    m_session.adoptWorld(std::move(world), std::move(player), slot, info.size, info.name, info.seed, info.difficulty);
    m_session.setDayTime(dayTime);

    m_mobSpawner.reset();
    m_session.getMinimap().rebuild(m_session.getWorld());
    m_session.setMinimapVisible(m_settingsMenu.getShowMinimap());

    m_camera.update(m_session.getPlayer());

    m_spawnGuard = 0;
    return true;
}

void Game::saveGame() {
    int slot = m_session.getCurrentSlot();
    if (slot < 0) return;
    if (!SaveManager::saveSlot(slot, m_session.getWorld(), m_session.getPlayer(),
                               m_session.getWorldName(), m_session.getWorldSize(),
                               m_session.getSeed(), m_session.getDifficulty(),
                               m_session.getDayTime()))
        std::cerr << "Failed to save game to slot " << slot << "!" << std::endl;
    else
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
    if (m_target.id > 0) UnloadRenderTexture(m_target);
    CloseWindow();
}
