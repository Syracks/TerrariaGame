#include "Game.hpp"
#include "core/Constants.hpp"
#include "core/Input.hpp"
#include "core/Math.hpp"
#include "core/TextureManager.hpp"
#include "core/SoundManager.hpp"
#include "items/Tool.hpp"
#include "systems/PhysicsSystem.hpp"
#include "systems/CollisionSystem.hpp"
#include "systems/MiningSystem.hpp"
#include "systems/RenderSystem.hpp"
#include "ui/HUD.hpp"
#include "ui/Minimap.hpp"
#include "save/SaveManager.hpp"
#include "world/TileRegistry.hpp"

#include <raylib.h>
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>
#include <algorithm>

namespace {
    constexpr float DAY_LENGTH = 300.0f;
    constexpr float NIGHT_LENGTH = 120.0f;
    constexpr float CYCLE_LENGTH = DAY_LENGTH + NIGHT_LENGTH;
    constexpr float NIGHT_START = DAY_LENGTH / CYCLE_LENGTH;
    constexpr int ZOMBIE_SPAWN_INTERVAL = 8;

    bool isSolidForSpawn(const World& world, int x, int y) {
        if (!world.isInBounds(x, y)) return false;

        TileId id = world.getTile(x, y);
        if (id == TileId::Air) return false;

        return TileRegistry::instance().get(id).solid;
    }

    bool hasHeadRoom(const World& world, int x, int groundY) {
        return !isSolidForSpawn(world, x, groundY - 1) &&
               !isSolidForSpawn(world, x, groundY - 2) &&
               !isSolidForSpawn(world, x, groundY - 3) &&
               !isSolidForSpawn(world, x, groundY - 4);
    }

    bool hasRealGroundBelow(const World& world, int x, int y) {
        int solidCount = 0;
        int maxY = std::min(world.getWorldHeight(), y + 30);

        for (int ty = y; ty < maxY; ++ty) {
            if (isSolidForSpawn(world, x, ty)) {
                solidCount++;
            }
        }

        return solidCount >= 20;
    }

    Vector2 findSafeSpawnPosition(const World& world) {
        int centerX = world.getWorldWidth() / 2;

        for (int radius = 0; radius <= 100; ++radius) {
            int candidates[2] = {
                centerX - radius,
                centerX + radius
            };

            for (int i = 0; i < 2; ++i) {
                int x = candidates[i];

                if (x < 2 || x >= world.getWorldWidth() - 2) continue;

                int startY = world.getWorldHeight() / 5;

                for (int y = startY; y < world.getWorldHeight() - 5; ++y) {
                    bool validGround =
                        isSolidForSpawn(world, x, y) &&
                        isSolidForSpawn(world, x, y + 1);

                    if (!validGround) continue;
                    if (!hasHeadRoom(world, x, y)) continue;
                    if (!hasRealGroundBelow(world, x, y)) continue;

                    return {
                        static_cast<float>(x * constants::TILE_SIZE),
                        static_cast<float>((y - 3) * constants::TILE_SIZE)
                    };
                }
            }
        }

        return {
            static_cast<float>(centerX * constants::TILE_SIZE),
            0.0f
        };
    }
}

Game::Game()
    : m_state(GameState::MainMenu)
    , m_world(std::make_unique<World>())
    , m_player(std::make_unique<Player>())
    , m_minimap(std::make_unique<Minimap>())
    , m_seed(static_cast<unsigned int>(std::time(nullptr))) {
}

Game::~Game() = default;

void Game::init() {
    InitWindow(constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, "Terraria");
    SetTargetFPS(constants::TARGET_FPS);
    SetExitKey(0);

    InitAudioDevice();
    SoundManager::instance().loadAll();

    Image img = LoadImage("assets/textures/backgrounds/forest.png");
    m_background = LoadTextureFromImage(img);
    UnloadImage(img);

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

        if (m_state == GameState::MainMenu || m_state == GameState::Settings) {
            ClearBackground(Color{20, 20, 30, 255});
        } else {
            float t = m_dayTime / CYCLE_LENGTH;
            Color sky;
            if (t < NIGHT_START) {
                float p = t / NIGHT_START;
                sky = {
                    static_cast<unsigned char>(135 + p * 50),
                    static_cast<unsigned char>(206 + p * 30),
                    static_cast<unsigned char>(235 + p * 10),
                    255
                };
            } else {
                float p = (t - NIGHT_START) / (1.0f - NIGHT_START);
                sky = {
                    static_cast<unsigned char>(185 - p * 130),
                    static_cast<unsigned char>(236 - p * 176),
                    static_cast<unsigned char>(245 - p * 185),
                    255
                };
            }
            ClearBackground(sky);
        }

        if (m_state == GameState::MainMenu) {
            m_menu.render();
        } else if (m_state == GameState::Settings) {
            m_settingsMenu.render();
        } else if (m_state == GameState::Playing || m_state == GameState::Inventory) {
            float scaleX = static_cast<float>(constants::SCREEN_WIDTH) / m_background.width;
            float scaleY = static_cast<float>(constants::SCREEN_HEIGHT) / m_background.height;
            float scale = (scaleX > scaleY) ? scaleX : scaleY;
            float bgW = m_background.width * scale;
            float bgH = m_background.height * scale;
            float bgX = (constants::SCREEN_WIDTH - bgW) / 2.0f;
            float bgY = (constants::SCREEN_HEIGHT - bgH) / 2.0f;
            float nightAlpha = 0.0f;
            float t = m_dayTime / CYCLE_LENGTH;
            if (t > NIGHT_START) {
                float p = (t - NIGHT_START) / (1.0f - NIGHT_START);
                nightAlpha = p;
            }
            Color bgTint = {
                static_cast<unsigned char>(255 * (1.0f - nightAlpha * 0.3f)),
                static_cast<unsigned char>(255 * (1.0f - nightAlpha * 0.5f)),
                static_cast<unsigned char>(255 * (1.0f - nightAlpha * 0.6f)),
                255
            };
            DrawTextureEx(m_background, {bgX, bgY}, 0.0f, scale, bgTint);
            Camera2D cam = m_camera.getCamera();
            cam.target.x = std::round(cam.target.x);
            cam.target.y = std::round(cam.target.y);
            BeginMode2D(cam);
            RenderSystem::renderWorld(*m_world, m_camera.getCamera(), nightAlpha);
            for (auto& mob : m_mobs) {
                mob->render();
            }
            m_particles.render();
            m_player->render();

            Vector2 playerCenter = {
                m_player->getPosition().x + m_player->getBounds().width / 2.0f,
                m_player->getPosition().y + m_player->getBounds().height / 2.0f
            };

            RenderSystem::renderLightingOverlay(*m_world,
                                                cam,
                                                nightAlpha,
                                                playerCenter);

            EndMode2D();

            HUD::render(*m_player);

            m_minimap->render(*m_world, playerCenter, m_minimapVisible);

            if (m_state == GameState::Inventory) {
                m_inventoryScreen.render(*m_player);
            }

            if (m_deathTimer > 0.0f) {
                float alpha = std::min(m_deathTimer / 2.0f * 200.0f, 180.0f);
                DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                              Color{180, 0, 0, static_cast<unsigned char>(alpha)});
                const char* deathText = "YOU DIED";
                int textSize = 60;
                int textW = MeasureText(deathText, textSize);
                DrawText(deathText, (constants::SCREEN_WIDTH - textW) / 2,
                         constants::SCREEN_HEIGHT / 2 - 30, textSize, WHITE);
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
        }
        return;
    }

    if (m_state == GameState::Inventory) {
        auto action = m_inventoryScreen.update(*m_player);
        if (action == InventoryScreen::Action::ReturnToMenu) {
            cleanupWorld();
            m_state = GameState::MainMenu;
            m_menu.setVisible(true);
        }
        return;
    }

    if (m_state != GameState::Playing)
        return;

    if (input::isMinimapToggled()) {
        m_minimapVisible = !m_minimapVisible;
    }

    int slot = input::getHotbarSelection();
    if (slot >= 0) m_player->getInventory().selectSlot(slot);

    if (input::isSavePressed()) saveGame();
}

void Game::update(float dt) {
    m_dayTime += dt;
    if (m_dayTime >= CYCLE_LENGTH) m_dayTime -= CYCLE_LENGTH;

    if (m_player->getHealth() > 0) {
        m_playerController.update(*m_player);
        if (input::isJumpPressed() && m_player->isOnGround()) {
            SoundManager::instance().play(SoundManager::Jump);
        }
        m_player->update(dt);
        CollisionSystem::resolveCollision(*m_player, *m_world, dt);
    }

    Vector2 playerCenter = {
        m_player->getPosition().x + m_player->getBounds().width / 2,
        m_player->getPosition().y + m_player->getBounds().height / 2
    };

    for (auto& mob : m_mobs) {
        mob->setPlayerPos(playerCenter);
        mob->update(dt);
        PhysicsSystem::update(*mob, *m_world, dt);
    }

    m_camera.update(*m_player);

    if (m_player->getHealth() <= 0 && m_deathTimer <= 2.0f) {
        m_player->setVelocity({0, 0});
    }

    if (m_player->getHealth() <= 0) {
        if (m_deathTimer <= 0.0f) {
            m_deathTimer = 2.0f;
            SoundManager::instance().play(SoundManager::PlayerDeath);
            float px = m_player->getPosition().x + m_player->getBounds().width / 2;
            float py = m_player->getPosition().y + m_player->getBounds().height / 2;
            for (int i = 0; i < 40; ++i) {
                float dx = (std::rand() % 600 - 300) * 1.2f;
                float dy = (std::rand() % 600 - 300) * 1.2f;
                Color c = {(unsigned char)(150 + std::rand() % 105),
                           (unsigned char)(std::rand() % 100),
                           (unsigned char)(std::rand() % 60), 255};
                m_particles.emit({px, py}, {dx, dy}, c, 0.8f + (std::rand() % 5) * 0.1f, 5, 2);
            }
        }
        m_deathTimer -= dt;
        if (m_deathTimer <= 0.0f) {
            Vector2 spawnPos = findSafeSpawnPosition(*m_world);
            m_player->setPosition(spawnPos);
            m_player->setVelocity({0, 0});
            m_player->heal(m_player->getMaxHealth());
            m_mobs.clear();
            spawnSlimes();
            m_deathTimer = 0.0f;
        }
        return;
    }

    if (input::isMinePressed()) {
        auto* sel = m_player->getInventory().getSelectedSlot();
        TileId held = (sel && sel->count > 0) ? sel->tileId : TileId::Air;

        if (held == TileId::Sword) {
            m_player->startSwing();
            SoundManager::instance().play(SoundManager::SwordSwing);
        } else if (isTool(held)) {
            Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), m_camera.getCamera());
            int tx = math::worldToTileX(worldPos.x);
            int ty = math::worldToTileY(worldPos.y);
            if (m_world->isInBounds(tx, ty)) {
                TileId target = m_world->getTile(tx, ty);
                if (target != TileId::Air) {
                    float mineTime = MiningSystem::getMiningTime(target, held);
                    if (mineTime > 0.0f) {
                        m_player->setMiningTarget(tx, ty);
                        m_player->startSwing(mineTime);
                    }
                } else if (held == TileId::Pickaxe && m_world->getWall(tx, ty) != TileId::Air) {
                    m_player->setMiningTarget(tx, ty);
                    m_player->startSwing(0.15f);
                }
            }
        }
    }

    if (m_player->isSwinging()) {
        auto* sel = m_player->getInventory().getSelectedSlot();
        TileId held = (sel && sel->count > 0) ? sel->tileId : TileId::Air;

        if (held == TileId::Sword) {
            Rectangle hitbox = m_player->getSwingHitbox();
            if (hitbox.width > 0 && hitbox.height > 0) {
                for (auto& mob : m_mobs) {
                    if (CheckCollisionRecs(hitbox, mob->getBounds())) {
                        mob->takeDamage(20);
                        SoundManager::instance().play(SoundManager::SwordHit);
                        for (int i = 0; i < 5; ++i) {
                            float px = mob->getBounds().x + mob->getBounds().width / 2;
                            float py = mob->getBounds().y + mob->getBounds().height / 2;
                            m_particles.emit({px, py}, {0, -100}, {255, 50, 50, 255}, 0.4f, 3, 1);
                        }
                        Vector2 c{hitbox.x + hitbox.width/2, hitbox.y + hitbox.height/2};
                        Vector2 pc{m_player->getBounds().x + m_player->getBounds().width/2,
                                   m_player->getBounds().y + m_player->getBounds().height/2};
                        Vector2 dir{c.x - pc.x, c.y - pc.y};
                        float len = std::sqrt(dir.x*dir.x + dir.y*dir.y);
                        if (len > 0.001f) { dir.x /= len; dir.y /= len; }
                        mob->knockback(dir);
                    }
                }
            }
        }
    }

    for (auto& mob : m_mobs) {
        if (m_player->getHealth() > 0 &&
            CheckCollisionRecs(m_player->getBounds(), mob->getBounds())) {
            m_player->takeDamage(mob->getContactDamage());
        }
    }

    for (auto it = m_mobs.begin(); it != m_mobs.end(); ) {
        if ((*it)->getHealth() <= 0) {
            auto& mob = *it;
            SoundManager::instance().play(SoundManager::MobDeath);
            for (int i = 0; i < 8; ++i) {
                float px = mob->getBounds().x + mob->getBounds().width / 2;
                float py = mob->getBounds().y + mob->getBounds().height / 2;
                m_particles.emit({px, py}, {0, -200}, {150, 150, 150, 255}, 0.6f, 4, 1);
            }
            mob->unload();
            it = m_mobs.erase(it);
        } else {
            ++it;
        }
    }

    m_particles.update(dt);

    float t = m_dayTime / CYCLE_LENGTH;
    if (t > NIGHT_START && m_mobs.size() < 20) {
        static int zombieSpawnTimer = 0;
        zombieSpawnTimer++;
        if (zombieSpawnTimer >= ZOMBIE_SPAWN_INTERVAL * constants::TARGET_FPS) {
            zombieSpawnTimer = 0;
            if (std::rand() % 3 == 0) spawnZombies();
        }
    }

    if (m_player->wasSwingJustCompleted()) {
        auto* sel = m_player->getInventory().getSelectedSlot();
        TileId held = (sel && sel->count > 0) ? sel->tileId : TileId::Air;
        if ((held == TileId::Pickaxe || held == TileId::Axe) &&
            m_player->getMiningTargetX() >= 0) {
            int tx = m_player->getMiningTargetX();
            int ty = m_player->getMiningTargetY();
            TileId mined = m_world->getTile(tx, ty);
            if (mined != TileId::Air) {
                MiningSystem::tryMineTile(*m_world, *m_player, tx, ty);
                if (m_world->getTile(tx, ty) == TileId::Air && mined != TileId::Air) {
                    SoundManager::instance().play(
                        held == TileId::Pickaxe ? SoundManager::PickaxeMine : SoundManager::AxeMine);
                    float cx = math::tileToWorldX(tx) + constants::TILE_SIZE / 2.0f;
                    float cy = math::tileToWorldY(ty) + constants::TILE_SIZE / 2.0f;
                    auto& reg = TileRegistry::instance();
                    Color c = reg.get(mined).color;
                    for (int i = 0; i < 6; ++i) {
                        m_particles.emit({cx, cy}, {0, -150}, c, 0.5f, 4, 1);
                    }
                    m_minimap->markDirty();
                }
            } else {
                TileId wall = m_world->getWall(tx, ty);
                if (wall != TileId::Air && held == TileId::Pickaxe) {
                    m_world->setWall(tx, ty, TileId::Air);
                    m_player->getInventory().addItem(wall, 1);
                    SoundManager::instance().play(SoundManager::PickaxeMine);
                    float cx = math::tileToWorldX(tx) + constants::TILE_SIZE / 2.0f;
                    float cy = math::tileToWorldY(ty) + constants::TILE_SIZE / 2.0f;
                    auto& reg = TileRegistry::instance();
                    Color c = reg.get(wall).color;
                    for (int i = 0; i < 4; ++i) {
                        m_particles.emit({cx, cy}, {0, -120}, c, 0.4f, 3, 1);
                    }
                    m_minimap->markDirty();
                }
            }
            m_player->clearMiningTarget();
        }
    }

    if (input::isPlacePressed()) {
        auto* sel = m_player->getInventory().getSelectedSlot();
        if (sel && sel->count > 0) {
            if (sel->tileId == TileId::Torch) {
                Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), m_camera.getCamera());
                int tx = math::worldToTileX(worldPos.x);
                int ty = math::worldToTileY(worldPos.y);
                if (m_world->isInBounds(tx, ty) && m_world->getTile(tx, ty) == TileId::Air) {
                    bool adjacent = false;
                    static const int dx[] = {0, 0, -1, 1};
                    static const int dy[] = {-1, 1, 0, 0};
                    for (int i = 0; i < 4; ++i) {
                        int nx = tx + dx[i];
                        int ny = ty + dy[i];
                        if (m_world->isInBounds(nx, ny)) {
                            TileId nid = m_world->getTile(nx, ny);
                            if (nid != TileId::Air && TileRegistry::instance().get(nid).solid) {
                                adjacent = true;
                                break;
                            }
                        }
                    }
                    if (adjacent) {
                        m_world->setTile(tx, ty, TileId::Torch);
                        m_player->getInventory().removeItem(TileId::Torch, 1);
                        SoundManager::instance().play(SoundManager::TorchPlace);
                        m_minimap->markDirty();
                    }
                }
            } else if (isWallItem(sel->tileId)) {
                Vector2 worldPos = GetScreenToWorld2D(GetMousePosition(), m_camera.getCamera());
                int tx = math::worldToTileX(worldPos.x);
                int ty = math::worldToTileY(worldPos.y);
                if (m_world->isInBounds(tx, ty) && m_world->getTile(tx, ty) == TileId::Air) {
                    if (m_world->getWall(tx, ty) == TileId::Air) {
                        bool adjacent = false;
                        static const int dx[] = {0, 0, -1, 1};
                        static const int dy[] = {-1, 1, 0, 0};
                        for (int i = 0; i < 4; ++i) {
                            int nx = tx + dx[i];
                            int ny = ty + dy[i];
                            if (m_world->isInBounds(nx, ny)) {
                                TileId nid = m_world->getTile(nx, ny);
                                if (nid != TileId::Air && TileRegistry::instance().get(nid).solid) {
                                    adjacent = true;
                                    break;
                                }
                            }
                        }
                        if (adjacent) {
                            m_world->setWall(tx, ty, sel->tileId);
                            m_player->getInventory().removeItem(sel->tileId, 1);
                            SoundManager::instance().play(SoundManager::BlockPlace);
                            m_minimap->markDirty();
                        }
                    }
                }
            } else {
                int oldCount = sel->count;
                MiningSystem::tryPlace(*m_world, *m_player, m_camera.getCamera());
                if (sel->count < oldCount) {
                    SoundManager::instance().play(SoundManager::BlockPlace);
                    m_minimap->markDirty();
                }
            }
        }
    }
}

void Game::render() {
}

void Game::spawnSlimes() {
    int count = 5 + std::rand() % 6;
    int worldWidth = m_world->getWorldWidth();
    int playerSpawnTx = worldWidth / 2;
    for (int i = 0; i < count; ++i) {
        int tx;
        int facing;
        int offset = 30 + std::rand() % 40;
        if (i % 2 == 0) {
            tx = playerSpawnTx - offset;
            facing = 1;
        } else {
            tx = playerSpawnTx + offset;
            facing = -1;
        }
        if (tx < 0 || tx >= worldWidth) continue;

        bool found = false;
        int surfaceY = 0;
        for (int y = 0; y < m_world->getWorldHeight(); ++y) {
            if (m_world->getTile(tx, y) != TileId::Air) {
                surfaceY = y;
                found = true;
                break;
            }
        }
        if (!found) continue;

        int spawnY = surfaceY - 2;
        if (spawnY < 0) spawnY = 0;
        MobType mt = (std::rand() % 3 == 0) ? MobType::BlueSlime : MobType::Slime;
        auto slime = std::make_unique<Mob>(mt, Vector2{
            static_cast<float>(tx) * constants::TILE_SIZE,
            static_cast<float>(spawnY) * constants::TILE_SIZE
        });
        slime->setFacing(facing);
        slime->load();
        m_mobs.push_back(std::move(slime));
    }
}

void Game::spawnZombies() {
    int count = 1 + std::rand() % 3;
    int playerTx = math::worldToTileX(m_player->getPosition().x + m_player->getBounds().width / 2);
    int worldWidth = m_world->getWorldWidth();
    for (int i = 0; i < count; ++i) {
        int tx;
        int facing;
        int offset = 40 + std::rand() % 30;
        if (i % 2 == 0) {
            tx = playerTx - offset;
            facing = 1;
        } else {
            tx = playerTx + offset;
            facing = -1;
        }
        if (tx < 0 || tx >= worldWidth) continue;

        int surfaceY = 0;
        bool found = false;
        for (int y = 0; y < m_world->getWorldHeight(); ++y) {
            if (m_world->getTile(tx, y) != TileId::Air) {
                surfaceY = y;
                found = true;
                break;
            }
        }
        if (!found) continue;

        int spawnY = surfaceY - 2;
        if (spawnY < 0) spawnY = 0;
        auto zombie = std::make_unique<Mob>(MobType::Zombie, Vector2{
            static_cast<float>(tx) * constants::TILE_SIZE,
            static_cast<float>(spawnY) * constants::TILE_SIZE
        });
        zombie->setFacing(facing);
        zombie->load();
        m_mobs.push_back(std::move(zombie));
    }
}

void Game::newGame(const std::string& name, WorldSize size, int slot) {
    WorldDimensions dims = getWorldDimensions(size);
    constants::WORLD_WIDTH = dims.width;
    constants::WORLD_HEIGHT = dims.height;
    m_worldSize = size;
    m_worldName = name;
    m_currentSlot = slot;

    m_world = std::make_unique<World>();
    m_player = std::make_unique<Player>();
    m_player->load();
    m_mobs.clear();
    m_particles.clear();
    m_deathTimer = 0.0f;
    m_dayTime = 0.0f;
    m_seed = static_cast<unsigned int>(std::time(nullptr));

    
    BeginDrawing();
    ClearBackground(Color{20, 20, 30, 255});
    const char* loadingTitle = "Generating World...";
    int loadingSize = 40;
    int loadingW = MeasureText(loadingTitle, loadingSize);
    DrawText(loadingTitle, (constants::SCREEN_WIDTH - loadingW) / 2,
             constants::SCREEN_HEIGHT / 2 - 80, loadingSize, GREEN);

    int barW = 400;
    int barH = 28;
    int barX = (constants::SCREEN_WIDTH - barW) / 2;
    int barY = constants::SCREEN_HEIGHT / 2 - 10;
    DrawRectangle(barX, barY, barW, barH, Color{40, 40, 50, 255});
    DrawRectangleLines(barX, barY, barW, barH, Color{80, 80, 120, 255});
    const char* phaseText = "Preparing...";
    DrawText(phaseText, (constants::SCREEN_WIDTH - MeasureText(phaseText, 20)) / 2,
             barY + barH + 12, 20, Color{120, 120, 140, 255});
    EndDrawing();

    m_world->generate(m_seed, [&](float progress) {
        BeginDrawing();
        ClearBackground(Color{20, 20, 30, 255});
        DrawText(loadingTitle, (constants::SCREEN_WIDTH - loadingW) / 2,
                 constants::SCREEN_HEIGHT / 2 - 80, loadingSize, GREEN);

        DrawRectangle(barX, barY, barW, barH, Color{40, 40, 50, 255});
        DrawRectangleLines(barX, barY, barW, barH, Color{80, 80, 120, 255});

        int fillW = static_cast<int>((barW - 4) * progress);
        if (fillW > 0)
            DrawRectangle(barX + 2, barY + 2, fillW, barH - 4, GREEN);

        const char* phases[] = {
            "Terrain...", "Caves...", "Cabins...",
            "Ores...", "Pockets...", "Trees...", "Islands..."
        };
        int phaseIdx = static_cast<int>(progress * 7);
        if (phaseIdx >= 7) phaseIdx = 6;
        const char* phase = phases[phaseIdx];
        DrawText(phase, (constants::SCREEN_WIDTH - MeasureText(phase, 20)) / 2,
                 barY + barH + 12, 20, Color{120, 120, 140, 255});

        std::string pct = std::to_string(static_cast<int>(progress * 100)) + "%";
        DrawText(pct.c_str(), barX + barW + 10, barY + 4, 20, GREEN);

        EndDrawing();
    });

    m_minimap->rebuild(*m_world);

    spawnSlimes();

    Vector2 spawnPos = findSafeSpawnPosition(*m_world);
    m_player->setPosition(spawnPos);
    m_player->setVelocity({0, 0});

    m_camera.update(*m_player);

    saveGame();
}

void Game::loadGame(int slot) {
    std::string dataPath = SaveManager::getSlotDataPath(slot);
    if (!SaveManager::saveExists(dataPath)) {
        return;
    }

    SlotInfo info = SaveManager::getSlotInfo(slot);
    WorldDimensions dims = getWorldDimensions(info.size);
    constants::WORLD_WIDTH = dims.width;
    constants::WORLD_HEIGHT = dims.height;
    m_worldSize = info.size;
    m_worldName = info.name;
    m_currentSlot = slot;
    m_seed = info.seed;

    m_world = std::make_unique<World>();
    m_player = std::make_unique<Player>();
    m_player->load();
    m_minimap = std::make_unique<Minimap>();
    SaveManager::loadSlot(slot, *m_world, *m_player);
    m_minimap->rebuild(*m_world);
    m_particles.clear();
    m_camera.update(*m_player);
}

void Game::saveGame() {
    if (m_currentSlot < 0) return;
    SaveManager::saveSlot(m_currentSlot, *m_world, *m_player, m_worldName, m_worldSize, m_seed);
    std::cout << "Game saved to slot " << m_currentSlot << "." << std::endl;
}

void Game::cleanupWorld() {
    m_mobs.clear();
    m_particles.clear();
    m_world = std::make_unique<World>();
    m_player = std::make_unique<Player>();
    m_minimap = std::make_unique<Minimap>();
    m_deathTimer = 0.0f;
    m_dayTime = 0.0f;
}

void Game::cleanup() {
    m_player->unload();
    for (auto& mob : m_mobs) {
        mob->unload();
    }
    m_mobs.clear();
    UnloadTexture(m_background);
    TextureManager::instance().unloadAll();
    SoundManager::instance().unloadAll();
    CloseAudioDevice();
    CloseWindow();
}
