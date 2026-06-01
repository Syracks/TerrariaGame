#include "MobSpawner.hpp"
#include "world/World.hpp"
#include "world/TileRegistry.hpp"
#include "entities/Player.hpp"
#include "core/Constants.hpp"
#include "core/Math.hpp"

namespace {
    constexpr float DAY_LENGTH = 300.0f;
    constexpr float NIGHT_LENGTH = 120.0f;
    constexpr float CYCLE_LENGTH = DAY_LENGTH + NIGHT_LENGTH;
    constexpr float NIGHT_START = 0.5f;
    constexpr int ZOMBIE_SPAWN_INTERVAL = 8;

    bool isSolidForSpawn(const World& world, int x, int y) {
        if (!world.isInBounds(x, y)) return false;
        TileId id = world.getTile(x, y);
        if (id == TileId::Air) return false;
        auto& reg = TileRegistry::instance();
        return reg.get(id).solid;
    }

    bool hasHeadRoom(const World& world, int x, int groundY) {
        for (int dy = 1; dy <= 4; ++dy) {
            if (isSolidForSpawn(world, x, groundY - dy)) return false;
        }
        return true;
    }

    bool hasRealGroundBelow(const World& world, int x, int y) {
        int solidCount = 0;
        int maxY = std::min(world.getWorldHeight(), y + 30);
        for (int ty = y; ty < maxY; ++ty) {
            if (isSolidForSpawn(world, x, ty)) solidCount++;
        }
        return solidCount >= 20;
    }

    int findSurfaceY(const World& world, int tx) {
        for (int y = 0; y < world.getWorldHeight(); ++y) {
            if (world.getTile(tx, y) != TileId::Air) return y;
        }
        return world.getWorldHeight() - 1;
    }
}

void MobSpawner::spawnSlimes(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                              const Player& player, std::mt19937& rng) {
    std::uniform_int_distribution<int> distCount(5, 10);
    std::uniform_int_distribution<int> distOff(30, 40);
    std::uniform_int_distribution<int> distType(0, 2);
    int count = distCount(rng);

    int worldWidth = world.getWorldWidth();
    int playerSpawnTx = worldWidth / 2;

    for (int i = 0; i < count; ++i) {
        int offset = distOff(rng);
        int tx = (i % 2 == 0) ? playerSpawnTx - offset : playerSpawnTx + offset;
        int facing = (i % 2 == 0) ? 1 : -1;

        if (tx < 0 || tx >= worldWidth) continue;

        int surfaceY = findSurfaceY(world, tx);
        if (surfaceY >= world.getWorldHeight()) continue;

        int spawnY = std::max(0, surfaceY - 2);
        MobType mt = (distType(rng) == 0) ? MobType::BlueSlime : MobType::Slime;

        auto slime = std::make_unique<Mob>(mt, Vector2{
            static_cast<float>(tx) * constants::TILE_SIZE,
            static_cast<float>(spawnY) * constants::TILE_SIZE
        });
        slime->setFacing(facing);
        slime->load();
        mobs.push_back(std::move(slime));
    }
}

void MobSpawner::spawnZombies(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                               const Player& player, std::mt19937& rng) {
    std::uniform_int_distribution<int> distCount(1, 3);
    std::uniform_int_distribution<int> distOff(40, 70);
    int count = distCount(rng);

    int playerTx = math::worldToTileX(player.getPosition().x + player.getBounds().width / 2);
    int worldWidth = world.getWorldWidth();

    for (int i = 0; i < count; ++i) {
        int offset = distOff(rng);
        int tx = (i % 2 == 0) ? playerTx - offset : playerTx + offset;
        int facing = (i % 2 == 0) ? 1 : -1;

        if (tx < 0 || tx >= worldWidth) continue;

        int surfaceY = findSurfaceY(world, tx);
        if (surfaceY >= world.getWorldHeight()) continue;

        int spawnY = std::max(0, surfaceY - 2);
        auto zombie = std::make_unique<Mob>(MobType::Zombie, Vector2{
            static_cast<float>(tx) * constants::TILE_SIZE,
            static_cast<float>(spawnY) * constants::TILE_SIZE
        });
        zombie->setFacing(facing);
        zombie->load();
        mobs.push_back(std::move(zombie));
    }
}

void MobSpawner::updateNightSpawning(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                                      const Player& player, float dayTime, float dt,
                                      std::mt19937& rng) {
    float t = dayTime / CYCLE_LENGTH;
    if (t <= NIGHT_START) return;
    if (mobs.size() >= 20) return;

    static int zombieSpawnTimer = 0;
    zombieSpawnTimer++;
    if (zombieSpawnTimer < ZOMBIE_SPAWN_INTERVAL * constants::TARGET_FPS) return;
    zombieSpawnTimer = 0;

    std::uniform_int_distribution<int> dist(0, 2);
    if (dist(rng) == 0) {
        spawnZombies(world, mobs, player, rng);
    }
}
