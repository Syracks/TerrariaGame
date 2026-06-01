#include "MobSpawner.hpp"
#include "world/World.hpp"
#include "world/TileRegistry.hpp"
#include "entities/Player.hpp"
#include "core/Constants.hpp"
#include "core/Math.hpp"

namespace {
    constexpr float CYCLE_LENGTH = 420.0f;
    constexpr float NIGHT_START = 0.5f;
    constexpr int maxSlimesTarget = 20;

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
                              const Player& player, std::mt19937& rng,
                              Difficulty difficulty) {
    bool hardcore = (difficulty == Difficulty::Hardcore);
    std::uniform_int_distribution<int> distCount(hardcore ? 8 : 3, hardcore ? 16 : 8);
    std::uniform_int_distribution<int> distX(20, world.getWorldWidth() - 20);
    std::uniform_int_distribution<int> distType(0, 4);
    int count = distCount(rng);

    int playerTx = math::worldToTileX(player.getPosition().x + player.getBounds().width / 2);
    int targetCount = std::min(maxSlimesTarget, static_cast<int>(mobs.size()) + count);

    for (int i = 0; i < count * 4; ++i) {
        if (static_cast<int>(mobs.size()) >= targetCount) break;

        int tx = distX(rng);
        if (std::abs(tx - playerTx) < 25) continue;
        if (tx < 0 || tx >= world.getWorldWidth()) continue;

        int surfaceY = findSurfaceY(world, tx);
        if (surfaceY >= world.getWorldHeight()) continue;
        if (!hasHeadRoom(world, tx, surfaceY)) continue;
        if (!hasRealGroundBelow(world, tx, surfaceY)) continue;

        int spawnY = std::max(0, surfaceY - 2);
        MobType mt = (distType(rng) == 0) ? MobType::BlueSlime : MobType::Slime;

        auto slime = std::make_unique<Mob>(mt, Vector2{
            static_cast<float>(tx) * constants::TILE_SIZE,
            static_cast<float>(spawnY) * constants::TILE_SIZE
        });
        slime->setFacing(distX(rng) < playerTx ? 1 : -1);
        slime->load();
        mobs.push_back(std::move(slime));
    }
}

void MobSpawner::spawnZombies(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                               const Player& player, std::mt19937& rng,
                               Difficulty difficulty) {
    bool hardcore = (difficulty == Difficulty::Hardcore);
    std::uniform_int_distribution<int> distCount(hardcore ? 3 : 1, hardcore ? 8 : 4);
    std::uniform_int_distribution<int> distOff(35, 80);
    int count = distCount(rng);

    int playerTx = math::worldToTileX(player.getPosition().x + player.getBounds().width / 2);
    int worldWidth = world.getWorldWidth();
    std::bernoulli_distribution sideDist(0.5);
    int targetCount = static_cast<int>(mobs.size()) + count;

    for (int i = 0; i < count * 4; ++i) {
        if (static_cast<int>(mobs.size()) >= targetCount) break;

        int offset = distOff(rng);
        int tx = sideDist(rng) ? playerTx - offset : playerTx + offset;
        if (tx < 0 || tx >= worldWidth) continue;

        int surfaceY = findSurfaceY(world, tx);
        if (surfaceY >= world.getWorldHeight()) continue;
        if (!hasHeadRoom(world, tx, surfaceY)) continue;
        if (!hasRealGroundBelow(world, tx, surfaceY)) continue;

        int spawnY = std::max(0, surfaceY - 2);
        auto zombie = std::make_unique<Mob>(MobType::Zombie, Vector2{
            static_cast<float>(tx) * constants::TILE_SIZE,
            static_cast<float>(spawnY) * constants::TILE_SIZE
        });
        zombie->setFacing(tx < playerTx ? 1 : -1);
        zombie->load();
        mobs.push_back(std::move(zombie));
    }
}

void MobSpawner::reset() {
    m_spawnTimer = 0.0f;
    m_nextSpawn = 0.0f;
}

void MobSpawner::updateNightSpawning(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                                      const Player& player, float dayTime, float dt,
                                      std::mt19937& rng,
                                      Difficulty difficulty) {
    float t = dayTime / CYCLE_LENGTH;
    if (t <= NIGHT_START) return;
    bool hardcore = (difficulty == Difficulty::Hardcore);
    int maxMobs = hardcore ? 40 : 20;
    if (static_cast<int>(mobs.size()) >= maxMobs) return;

    if (m_nextSpawn == 0.0f) {
        std::uniform_real_distribution<float> distDelay(hardcore ? 1.5f : 3.0f, hardcore ? 5.0f : 10.0f);
        m_nextSpawn = distDelay(rng);
    }

    m_spawnTimer += dt;
    if (m_spawnTimer < m_nextSpawn) return;
    m_spawnTimer = 0.0f;

    std::uniform_real_distribution<float> distDelay(hardcore ? 1.5f : 3.0f, hardcore ? 5.0f : 10.0f);
    m_nextSpawn = distDelay(rng);

    std::uniform_int_distribution<int> distType(0, 3);
    int roll = distType(rng);
    if (roll < 2) {
        spawnZombies(world, mobs, player, rng, difficulty);
    } else {
        spawnSlimes(world, mobs, player, rng, difficulty);
    }
}
