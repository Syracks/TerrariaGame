#include "DeathSystem.hpp"
#include "entities/Player.hpp"
#include "entities/Mob.hpp"
#include "world/World.hpp"
#include "world/TileRegistry.hpp"
#include "systems/ParticleSystem.hpp"
#include "core/Constants.hpp"
#include "core/Math.hpp"
#include "core/SoundManager.hpp"
#include <cmath>
#include <algorithm>
#include <fstream>

#ifdef DEBUG
#define DBG_LOG(x) do { std::ofstream log("/tmp/opencode_spawn_debug.log", std::ios::app); log << x << "\n"; } while(0)
#else
#define DBG_LOG(x)
#endif

Vector2 DeathSystem::findSafeSpawnPosition(const World& world) {
    int centerX = world.getWorldWidth() / 2;

    auto isSurfaceBlock = [&](int x, int y) -> bool {
        if (!world.isInBounds(x, y)) return false;
        TileId id = world.getTile(x, y);
        if (id == TileId::Air) return false;
        if (!TileRegistry::instance().get(id).solid) return false;
        if (!world.isInBounds(x, y - 1)) return false;
        return world.getTile(x, y - 1) == TileId::Air;
    };

    for (int x = centerX - 5; x <= centerX + 5; ++x) {
        if (x < 2 || x >= world.getWorldWidth() - 2) continue;

        for (int y = 0; y < world.getWorldHeight() - 2; ++y) {
            if (!isSurfaceBlock(x, y)) continue;
            float spawnY = static_cast<float>(y) * constants::TILE_SIZE - 32.0f;

            DBG_LOG("Spawn at tile (" << x << "," << y << ") spawnY=" << spawnY);

            return {
                static_cast<float>(x * constants::TILE_SIZE),
                spawnY
            };
        }
    }

    DBG_LOG("FALLBACK - no surface block found! centerX=" << centerX);

    return {
        static_cast<float>(centerX * constants::TILE_SIZE),
        0.0f
    };
}

bool DeathSystem::updateDeath(Player& player, World& world,
                               std::vector<std::unique_ptr<Mob>>& mobs,
                               float& deathTimer, float dt,
                               ParticleSystem& particles,
                               std::mt19937& rng) {
    if (player.getHealth() > 0) return false;

    if (deathTimer <= 0.0f) {
        deathTimer = 2.0f;
        SoundManager::instance().play(SoundManager::PlayerDeath);
        float px = player.getPosition().x + player.getBounds().width / 2;
        float py = player.getPosition().y + player.getBounds().height / 2;

        std::uniform_int_distribution<int> distC(0, 104);
        std::uniform_int_distribution<int> distOff(0, 599);
        for (int i = 0; i < 40; ++i) {
            float dx = (distOff(rng) - 300) * 1.2f;
            float dy = (distOff(rng) - 300) * 1.2f;
            Color c = {(unsigned char)(150 + distC(rng)),
                       (unsigned char)(distC(rng)),
                       (unsigned char)(distC(rng)), 255};
            particles.emit({px, py}, {dx, dy}, c, 0.8f + (distC(rng) % 5) * 0.1f, 5, 2);
        }
    }

    deathTimer -= dt;
    if (deathTimer <= 0.0f) {
        Vector2 spawnPos = findSafeSpawnPosition(world);
        player.setPosition(spawnPos);
        player.setVelocity({0, 0});
        player.heal(player.getMaxHealth());
        mobs.clear();

        int count = 5 + static_cast<int>(rng() % 6);
        for (int i = 0; i < count; ++i) {
            int offset = 30 + static_cast<int>(rng() % 40);
            int tx = (i % 2 == 0) ? (world.getWorldWidth() / 2 - offset)
                                  : (world.getWorldWidth() / 2 + offset);
            if (tx < 0 || tx >= world.getWorldWidth()) continue;

            int surfaceY = 0;
            for (int y = 0; y < world.getWorldHeight(); ++y) {
                if (world.getTile(tx, y) != TileId::Air) { surfaceY = y; break; }
            }
            if (surfaceY <= 0) continue;

            MobType mt = (rng() % 3 == 0) ? MobType::BlueSlime : MobType::Slime;
            auto slime = std::make_unique<Mob>(mt, Vector2{
                static_cast<float>(tx) * constants::TILE_SIZE,
                static_cast<float>(std::max(0, surfaceY - 2)) * constants::TILE_SIZE
            });
            slime->setFacing((i % 2 == 0) ? 1 : -1);
            slime->load();
            mobs.push_back(std::move(slime));
        }

        deathTimer = 0.0f;
        return true;
    }

    player.setVelocity({0, 0});
    return false;
}
