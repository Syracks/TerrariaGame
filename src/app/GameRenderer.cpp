#include "GameRenderer.hpp"
#include "GameSession.hpp"
#include "entities/Player.hpp"
#include "entities/Mob.hpp"
#include "ui/HUD.hpp"
#include "ui/Minimap.hpp"
#include "ui/InventoryScreen.hpp"
#include "ui/MainMenu.hpp"
#include "ui/SettingsMenu.hpp"
#include "world/World.hpp"
#include "world/Tile.hpp"
#include "core/Math.hpp"
#include "systems/RenderSystem.hpp"
#include "systems/ParticleSystem.hpp"
#include "core/TextureManager.hpp"

namespace {
    constexpr float CYCLE_LENGTH = 420.0f;
    constexpr int UNDERGROUND_THRESHOLD = 30;
    constexpr int BEACH_MARGIN = 60;
}

GameRenderer::GameRenderer() = default;

void GameRenderer::init() {
    m_forestBg = loadBackground("assets/textures/backgrounds/forest.png");
    m_desertBg = loadBackground("assets/textures/backgrounds/desert.png");
    m_snowBg = loadBackground("assets/textures/backgrounds/snow.png");
    m_jungleBg = loadBackground("assets/textures/backgrounds/jungle.png");
    m_undergroundBg = loadBackground("assets/textures/backgrounds/underground.png");
    m_beachBg = loadBackground("assets/textures/backgrounds/beach.png");
}

Texture2D GameRenderer::loadBackground(const char* path) {
    Image img = LoadImage(path);
    if (img.data == nullptr) return {};
    Texture2D tex = LoadTextureFromImage(img);
    UnloadImage(img);
    return tex;
}

void GameRenderer::unloadBackground(Texture2D& tex) {
    if (tex.id > 0) {
        UnloadTexture(tex);
        tex = {};
    }
}

void GameRenderer::cleanup() {
    unloadBackground(m_forestBg);
    unloadBackground(m_desertBg);
    unloadBackground(m_snowBg);
    unloadBackground(m_jungleBg);
    unloadBackground(m_undergroundBg);
    unloadBackground(m_beachBg);
}

void GameRenderer::renderMainMenu(MainMenu& menu) {
    ClearBackground(Color{20, 20, 30, 255});
    menu.render();
}

void GameRenderer::renderSettingsMenu(SettingsMenu& menu) {
    ClearBackground(Color{20, 20, 30, 255});
    menu.render();
}

void GameRenderer::renderSky(float dayTime) {
    float nightAmount = getNightAmount(dayTime);

    Color dayColor = {185, 236, 245, 255};
    Color nightColor = {20, 25, 55, 255};

    Color sky = {
        static_cast<unsigned char>(dayColor.r + (nightColor.r - dayColor.r) * nightAmount),
        static_cast<unsigned char>(dayColor.g + (nightColor.g - dayColor.g) * nightAmount),
        static_cast<unsigned char>(dayColor.b + (nightColor.b - dayColor.b) * nightAmount),
        255
    };
    ClearBackground(sky);
}

void GameRenderer::renderBackground(float dayTime, const World& world, const Player& player) {
    if (m_forestBg.id <= 0) return;

    float px = player.getPosition().x + player.getBounds().width / 2.0f;
    float py = player.getPosition().y + player.getBounds().height / 2.0f;
    int tileX = math::worldToTileX(px);
    int tileY = math::worldToTileY(py);

    const Texture2D* bg = &m_forestBg;

    int surfaceY = world.getSurfaceHeight(tileX);
    if (tileY > surfaceY + UNDERGROUND_THRESHOLD) {
        bg = &m_undergroundBg;
    } else {
        if (tileX < BEACH_MARGIN || tileX >= constants::WORLD_WIDTH - BEACH_MARGIN) {
            if (tileY <= surfaceY + 5) {
                bg = &m_beachBg;
            }
        }
        if (bg == &m_forestBg) {
            Biome biome = world.getBiome(tileX);
            switch (biome) {
                case Biome::Desert:   bg = &m_desertBg; break;
                case Biome::Snow:     bg = &m_snowBg;   break;
                case Biome::Jungle:   bg = &m_jungleBg; break;
                default: break;
            }
        }
    }

    if (bg->id <= 0) return;

    float scaleX = static_cast<float>(constants::SCREEN_WIDTH) / bg->width;
    float scaleY = static_cast<float>(constants::SCREEN_HEIGHT) / bg->height;
    float scale = (scaleX > scaleY) ? scaleX : scaleY;
    float bgW = bg->width * scale;
    float bgH = bg->height * scale;
    float bgX = (constants::SCREEN_WIDTH - bgW) / 2.0f;
    float bgY = (constants::SCREEN_HEIGHT - bgH) / 2.0f;

    float nightAmount = getNightAmount(dayTime);

    Color tint = {
        static_cast<unsigned char>(255 - nightAmount * 160),
        static_cast<unsigned char>(255 - nightAmount * 180),
        static_cast<unsigned char>(255 - nightAmount * 140),
        255
    };
    DrawTextureEx(*bg, {bgX, bgY}, 0.0f, scale, tint);
}

void GameRenderer::renderWorldAndEntities(GameSession& session, const Camera2D& camera) {
    Camera2D cam = camera;
    cam.target.x = std::round(cam.target.x);
    cam.target.y = std::round(cam.target.y);

    const auto& world = session.getWorld();
    const auto& player = session.getPlayer();
    const auto& mobs = session.getMobs();
    const auto& particles = session.getParticles();

    BeginMode2D(cam);
    RenderSystem::renderWorld(world, cam);
    for (auto& mob : mobs) {
        mob->render();
    }
    particles.render();
    player.render();

    Vector2 playerCenter = {
        player.getPosition().x + player.getBounds().width / 2.0f,
        player.getPosition().y + player.getBounds().height / 2.0f
    };

    RenderSystem::renderLightingOverlay(world, cam, playerCenter);
    EndMode2D();
}

void GameRenderer::renderHUD(GameSession& session) {
    HUD::render(session.getPlayer(), session.getDayTime());
}

void GameRenderer::renderMinimap(GameSession& session) {
    const auto& player = session.getPlayer();
    Vector2 playerCenter = {
        player.getPosition().x + player.getBounds().width / 2.0f,
        player.getPosition().y + player.getBounds().height / 2.0f
    };
    session.getMinimap().render(session.getWorld(), playerCenter, session.isMinimapVisible());
}

float GameRenderer::getNightAmount(float dayTime) {
    float t = dayTime / CYCLE_LENGTH;
    // t=0=8am dawn, t=0.5=8pm dusk, t=1.0=8am next dawn, peaks at t=0.75=2am midnight
    if (t <= 0.35f) return 0.0f;
    if (t <= 0.50f) return (t - 0.35f) / 0.15f * 0.4f;
    if (t <= 0.65f) return 0.4f + (t - 0.50f) / 0.15f * 0.6f;
    if (t <= 0.85f) return 1.0f;
    return (1.0f - t) / 0.15f;
}

void GameRenderer::renderNightOverlay(float dayTime) {
    float night = getNightAmount(dayTime);
    if (night <= 0.0f) return;

    unsigned char alpha = static_cast<unsigned char>(140 * night);
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                  Color{8, 15, 50, alpha});
}

void GameRenderer::renderDeathOverlay(float deathTimer) {
    if (deathTimer <= 0.0f) return;

    float alpha = std::min(deathTimer / 2.0f * 200.0f, 180.0f);
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                  Color{180, 0, 0, static_cast<unsigned char>(alpha)});
    const char* deathText = "YOU DIED";
    int textSize = 60;
    int textW = MeasureText(deathText, textSize);
    DrawText(deathText, (constants::SCREEN_WIDTH - textW) / 2,
             constants::SCREEN_HEIGHT / 2 - 30, textSize, WHITE);
}

void GameRenderer::renderGame(GameSession& session, const Camera2D& camera,
                               bool inventoryOpen, const InventoryScreen& inventoryScreen,
                               const World& world) {
    renderSky(session.getDayTime());
    renderBackground(session.getDayTime(), world, session.getPlayer());
    renderWorldAndEntities(session, camera);
    renderNightOverlay(session.getDayTime());
    renderHUD(session);

    renderMinimap(session);
    renderDeathOverlay(session.getDeathTimer());

    if (inventoryOpen) {
        inventoryScreen.render(session.getPlayer(), world);
    }
}
