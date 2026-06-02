#include "Game.hpp"
#include "core/Constants.hpp"
#include "core/Input.hpp"
#include "core/Math.hpp"
#include "core/TextureManager.hpp"
#include "core/SoundManager.hpp"
#include "core/MusicManager.hpp"
#include "items/ItemDefinition.hpp"
#include "items/Tool.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/LiquidSystem.hpp"
#include "systems/MiningSystem.hpp"
#include "ui/HUD.hpp"
#include "ui/Minimap.hpp"
#include "save/SaveManager.hpp"
#include "world/Tile.hpp"
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
    MusicManager::instance().loadAll();
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

        MusicManager::instance().tick();

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

    m_session.setMinimapVisible(true);
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
                m_prevState = GameState::MainMenu;
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
            m_state = m_prevState;
        }
        return;
    }

    if (input::isPausePressed()) {
        if (m_state == GameState::Playing) {
            MusicManager::instance().stop();
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
        } else if (action == InventoryScreen::Action::OpenSettings) {
            m_prevState = GameState::Inventory;
            m_settingsMenu.saveToFile();
            m_state = GameState::Settings;
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

    if (input::isSavePressed()) {
        bool bossAlive = false;
        for (auto& m : m_session.getMobs())
            if (m->isBoss() && m->getHealth() > 0) { bossAlive = true; break; }
        if (!bossAlive) saveGame();
    }

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
        auto* sel = player.getInventory().getSelectedSlot();
        if (sel && sel->count > 0 && sel->tileId == TileId::AncientSeed) {
            bool bossAlive = false;
            for (auto& m : m_session.getMobs()) {
                if (m->isBoss() && m->getHealth() > 0) { bossAlive = true; break; }
            }
            if (!bossAlive && !m_session.isBossSummonRequested()) {
                float px = player.getPosition().x + player.getBounds().width / 2;
                int tileX = math::worldToTileX(px);
                Biome biome = world.getBiome(tileX);
                if (biome == Biome::Forest) {
                    player.getInventory().removeItem(TileId::AncientSeed, 1);
                    m_session.requestBossSummon();
                    HUD::showMessage("The Forest Guardian has awakened!", 4.0f, 300);
                    SoundManager::instance().play(SoundManager::BossSummon);
                }
            }
            return;
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
        mob->setWorldPtr(&world);
        mob->update(dt);
        PhysicsSystem::update(*mob, world, dt);
    }

    if (m_session.isBossSummonRequested()) {
        m_session.clearBossSummon();
        float px = player.getPosition().x + player.getBounds().width / 2;
        int tileX = math::worldToTileX(px + 80.0f);
        int surfaceY = 0;
        for (int y = 0; y < world.getWorldHeight(); ++y) {
            if (world.getTile(tileX, y) != TileId::Air) { surfaceY = y; break; }
        }
        float groundY = static_cast<float>(surfaceY) * constants::TILE_SIZE;
        auto boss = std::make_unique<Mob>(MobType::ForestGuardian,
            Vector2{static_cast<float>(tileX) * constants::TILE_SIZE, groundY - 160.0f});
        boss->load();
        mobs.push_back(std::move(boss));
    }

    for (auto& mob : mobs) {
        if (!mob->isBoss()) continue;

        if (mob->isSummonPending()) {
            mob->clearSummonPending();
            auto& rng = m_session.getRNG();
            std::uniform_int_distribution<int> distCount(2, 3);
            int count = distCount(rng);
            for (int i = 0; i < count; ++i) {
                float sx = mob->getBounds().x + mob->getBounds().width / 2 + (i - 1) * 20.0f;
                float sy = mob->getBounds().y + mob->getBounds().height;
                auto slime = std::make_unique<Mob>(MobType::Slime, Vector2{sx, sy});
                slime->setPlayerPos(playerCenter);
                slime->load();
                slime->setFacing((i % 2 == 0) ? 1 : -1);
                mobs.push_back(std::move(slime));
            }
        }

        if (mob->isProjectilePending()) {
            mob->clearProjectilePending();
            auto& projectiles = m_session.getProjectiles();
            Rectangle mb = mob->getBounds();
            float sx = mb.x + mb.width / 2;
            float sy = mb.y + mb.height;
            float dx = playerCenter.x - sx;
            float dy = playerCenter.y - sy;
            bool isFireball = mob->getPendingProjectileType() == ProjectileType::Fireball;
            float speed = isFireball ? 600.0f : 400.0f;

            Projectile p;
            p.position = {sx, sy};
            p.type = mob->getPendingProjectileType();
            p.damage = 15;
            p.lifetime = 3.0f;
            p.fromBoss = true;

            if (isFireball) {
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.0f) { dx /= len; dy /= len; }
                p.velocity = {dx * speed, dy * speed};
                p.noGravity = true;
            } else if (std::abs(dx) > 1.0f) {
                float a = dy / dx;
                float g2 = 400.0f;
                float v2 = speed * speed;
                float A = 1.0f + a * a;
                float B = -(v2 + 800.0f * dy);
                float C = g2 * g2 * dx * dx;
                float disc = B * B - 4.0f * A * C;
                float vx = speed * (dx > 0.0f ? 1.0f : -1.0f);
                if (disc >= 0.0f) {
                    float uSol = (-B - std::sqrt(disc)) / (2.0f * A);
                    if (uSol > 0.0f) vx = std::sqrt(uSol) * (dx > 0.0f ? 1.0f : -1.0f);
                }
                p.velocity = {vx, a * vx - g2 * dx / vx};
            } else {
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.0f) { dx /= len; dy /= len; }
                p.velocity = {dx * speed, dy * speed};
            }
            projectiles.push_back(p);
        }

        if (mob->isMeleePending()) {
            mob->clearMeleePending();
            Rectangle mb = mob->getBounds();
            float swipeX = (mob->getFacing() == 1) ? mb.x + mb.width : mb.x - 32.0f;
            TempHitbox h;
            h.bounds = {swipeX, mb.y, 32.0f, mb.height};
            h.damage = 20;
            h.lifetime = 0.3f;
            h.targetsPlayer = true;
            m_session.getTempHitboxes().push_back(h);
        }
    }

    {
        Biome biome = Biome::Forest;
        if (!mobs.empty()) {
            float px = player.getPosition().x + player.getBounds().width / 2;
            biome = world.getBiome(math::worldToTileX(px));
        }
        bool bossAlive = false;
        for (auto& m : mobs) {
            if (m->isBoss() && m->getHealth() > 0) { bossAlive = true; break; }
        }
        MusicManager::instance().update(biome, bossAlive, m_session.getDayTime());
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
                Projectile proj;
                float centerX = player.getPosition().x + player.getBounds().width / 2;
                float centerY = player.getPosition().y + player.getBounds().height / 2;
                proj.position = {centerX, centerY - 4};
                proj.type = ProjectileType::Arrow;
                Vector2 mouse = math::getVirtualMouse();
                Vector2 worldPos = GetScreenToWorld2D(mouse, m_camera.getCamera());
                float dx = worldPos.x - centerX;
                float dy = worldPos.y - centerY;
                float len = std::sqrt(dx * dx + dy * dy);
                if (len > 0.1f) {
                    float arrowSpeed = 900.0f;
                    proj.velocity = {dx / len * arrowSpeed, dy / len * arrowSpeed};
                    proj.facing = dx < 0 ? -1 : 1;
                } else {
                    int facing = player.isFacingLeft() ? -1 : 1;
                    proj.velocity = {static_cast<float>(facing) * 900.0f, -50.0f};
                    proj.facing = facing;
                }
                {
                    auto* sel = player.getInventory().getSelectedSlot();
                    if (sel) proj.damage = ItemDatabase::instance().get(sel->tileId).tool.damage;
                    if (proj.damage <= 0) proj.damage = 10;
                }
                m_session.getProjectiles().push_back(proj);
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
                } else if (mob->getType() == MobType::ForestGuardian) {
                    player.getInventory().addItem(TileId::GoldBar, distCount(rng) + 4);
                    player.getInventory().addItem(TileId::Wood, distCount(rng) * 10 + 15);
                    player.getInventory().addItem(TileId::Gel, distCount(rng) * 5 + 8);
                    if (dist100(rng) < 50) {
                        player.getInventory().addItem(TileId::AncientSeed, 1);
                    }
                    HUD::showMessage("Forest Guardian defeated!", 4.0f, 300);
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
        auto& projectiles = m_session.getProjectiles();
        auto& particles = m_session.getParticles();
        const float GRAVITY = 800.0f;
        for (auto it = projectiles.begin(); it != projectiles.end(); ) {
            Projectile& p = *it;
            p.lifetime -= dt;
            if (!p.active || p.lifetime <= 0.0f) {
                it = projectiles.erase(it);
                continue;
            }
            if (!p.noGravity) p.velocity.y += GRAVITY * dt;
            p.position.x += p.velocity.x * dt;
            p.position.y += p.velocity.y * dt;

            int tx = math::worldToTileX(p.position.x);
            int ty = math::worldToTileY(p.position.y);
            bool hitTile = false;
            if (world.isInBounds(tx, ty)) {
                TileId tile = world.getTile(tx, ty);
                if (tile != TileId::Air && TileRegistry::instance().get(tile).solid) {
                    hitTile = true;
                }
            }
            if (p.position.x < 0 || p.position.x > constants::WORLD_WIDTH * constants::TILE_SIZE ||
                p.position.y < 0 || p.position.y > constants::WORLD_HEIGHT * constants::TILE_SIZE) {
                hitTile = true;
            }

            bool hitMob = false;
            if (p.fromBoss) {
                if (CheckCollisionPointRec(p.position, player.getBounds())) {
                    player.takeDamage(p.damage);
                    hitMob = true;
                }
            } else {
                for (auto& mob : mobs) {
                    Rectangle mobBounds = mob->getBounds();
                    if (CheckCollisionPointRec(p.position, mobBounds)) {
                        mob->takeDamage(p.damage);
                        for (int i = 0; i < 5; ++i) {
                            Color c = (p.type == ProjectileType::Fireball)
                                ? Color{255, 120, 40, 255}
                                : Color{255, 100, 50, 255};
                            particles.emit(p.position, {0, -100}, c, 0.4f, 3, 1);
                        }
                        hitMob = true;
                        break;
                    }
                }
            }

            if (hitTile || hitMob) {
                Color partColor{200, 180, 140, 255};
                if (p.type == ProjectileType::Fireball) partColor = {255, 150, 50, 255};
                else if (p.type == ProjectileType::Leaf) partColor = {80, 200, 60, 255};
                for (int i = 0; i < 4; ++i) {
                    particles.emit(p.position,
                        {p.velocity.x * 0.2f, p.velocity.y * 0.2f},
                        partColor, 0.5f, 3, 1);
                }
                it = projectiles.erase(it);
            } else {
                ++it;
            }
        }
    }

    {
        auto& hitboxes = m_session.getTempHitboxes();
        for (auto it = hitboxes.begin(); it != hitboxes.end(); ) {
            TempHitbox& h = *it;
            h.lifetime -= dt;
            if (!h.active || h.lifetime <= 0.0f) {
                it = hitboxes.erase(it);
                continue;
            }
            if (h.targetsPlayer) {
                if (CheckCollisionRecs(h.bounds, player.getBounds())) {
                    player.takeDamage(h.damage);
                    h.lifetime = 0.0f;
                }
            } else {
                for (auto& mob : mobs) {
                    if (CheckCollisionRecs(h.bounds, mob->getBounds())) {
                        mob->takeDamage(h.damage);
                        h.lifetime = 0.0f;
                        break;
                    }
                }
            }
            ++it;
        }
    }

    m_session.getParticles().update(dt);
    HUD::update(dt);

    Difficulty diff = m_session.getDifficulty();
    m_mobSpawner.updateNightSpawning(world, mobs, player, m_session.getDayTime(), dt, m_session.getRNG(), diff);

    m_autosaveTimer += dt;
    if (m_autosaveTimer >= AUTOSAVE_INTERVAL) {
        bool bossAlive = false;
        for (auto& m : mobs)
            if (m->isBoss() && m->getHealth() > 0) { bossAlive = true; break; }
        if (!bossAlive) saveGame();
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

    m_session.getMinimap().rebuild(world);

    m_mobSpawner.spawnSlimes(world, m_session.getMobs(), player, rng, difficulty);

    Vector2 spawnPos = DeathSystem::findSafeSpawnPosition(world);
    player.setPosition(spawnPos);
    player.setVelocity({0, 0});

    m_session.setMinimapVisible(true);

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
    m_session.setMinimapVisible(true);

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
    MusicManager::instance().stop();
}

void Game::cleanup() {
    m_session.clear();
    m_renderer.cleanup();
    TextureManager::instance().unloadAll();
    SoundManager::instance().unloadAll();
    MusicManager::instance().unloadAll();
    CloseAudioDevice();
    if (m_target.id > 0) UnloadRenderTexture(m_target);
    CloseWindow();
}
