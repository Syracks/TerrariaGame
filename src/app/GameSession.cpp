#include "GameSession.hpp"
#include "save/SaveManager.hpp"
#include "world/WorldGenerator.hpp"

namespace {
    constexpr float DAY_LENGTH = 210.0f;
    constexpr float NIGHT_LENGTH = 210.0f;
    constexpr float CYCLE_LENGTH = DAY_LENGTH + NIGHT_LENGTH;
}

GameSession::GameSession()
    : m_world(std::make_unique<World>())
    , m_player(std::make_unique<Player>())
    , m_minimap(std::make_unique<Minimap>())
    , m_rng(std::random_device{}())
{
}

void GameSession::advanceDayTime(float dt) {
    m_dayTime += dt;
    if (m_dayTime >= CYCLE_LENGTH) m_dayTime -= CYCLE_LENGTH;
}

void GameSession::newWorld(const std::string& name, WorldSize size, int slot, unsigned int seed, Difficulty difficulty) {
    WorldDimensions dims = getWorldDimensions(size);
    constants::WORLD_WIDTH = dims.width;
    constants::WORLD_HEIGHT = dims.height;
    m_worldSize = size;
    m_difficulty = difficulty;
    m_worldName = name;
    m_currentSlot = slot;
    m_seed = seed;
    m_rng.seed(seed);

    m_world = std::make_unique<World>();
    m_player = std::make_unique<Player>();
    m_player->load();
    m_mobs.clear();
    m_particles.clear();
    m_deathTimer = 0.0f;
    m_dayTime = 0.0f;
    m_minimap = std::make_unique<Minimap>();
}

void GameSession::adoptWorld(std::unique_ptr<World> world, std::unique_ptr<Player> player,
                              int slot, WorldSize size, const std::string& name, unsigned int seed,
                              Difficulty difficulty) {
    m_worldSize = size;
    m_difficulty = difficulty;
    m_worldName = name;
    m_currentSlot = slot;
    m_seed = seed;
    m_rng.seed(seed);

    m_world = std::move(world);
    m_player = std::move(player);
    m_minimap = std::make_unique<Minimap>();
    m_mobs.clear();
    m_particles.clear();
    m_deathTimer = 0.0f;
    m_dayTime = 0.0f;
}

void GameSession::clear() {
    m_mobs.clear();
    m_particles.clear();
    m_world = std::make_unique<World>();
    m_player = std::make_unique<Player>();
    m_minimap = std::make_unique<Minimap>();
    m_deathTimer = 0.0f;
    m_dayTime = 0.0f;
    m_currentSlot = -1;
}
