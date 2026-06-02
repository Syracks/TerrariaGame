#pragma once

#include "world/World.hpp"
#include "entities/Player.hpp"
#include "entities/Mob.hpp"
#include "entities/Arrow.hpp"
#include "entities/TempHitbox.hpp"
#include "systems/ParticleSystem.hpp"
#include "ui/Minimap.hpp"
#include "core/Constants.hpp"
#include <memory>
#include <vector>
#include <random>

class GameSession {
public:
    GameSession();

    void newWorld(const std::string& name, WorldSize size, int slot, unsigned int seed, Difficulty difficulty = Difficulty::Normal);
    void adoptWorld(std::unique_ptr<World> world, std::unique_ptr<Player> player,
                    int slot, WorldSize size, const std::string& name, unsigned int seed,
                    Difficulty difficulty = Difficulty::Normal);
    void clear();

    World& getWorld() { return *m_world; }
    const World& getWorld() const { return *m_world; }
    Player& getPlayer() { return *m_player; }
    const Player& getPlayer() const { return *m_player; }
    std::vector<std::unique_ptr<Mob>>& getMobs() { return m_mobs; }
    const std::vector<std::unique_ptr<Mob>>& getMobs() const { return m_mobs; }
    std::vector<Projectile>& getProjectiles() { return m_projectiles; }
    const std::vector<Projectile>& getProjectiles() const { return m_projectiles; }
    std::vector<TempHitbox>& getTempHitboxes() { return m_tempHitboxes; }
    const std::vector<TempHitbox>& getTempHitboxes() const { return m_tempHitboxes; }
    ParticleSystem& getParticles() { return m_particles; }
    const ParticleSystem& getParticles() const { return m_particles; }
    Minimap& getMinimap() { return *m_minimap; }
    const Minimap& getMinimap() const { return *m_minimap; }
    bool isMinimapVisible() const { return m_minimapVisible; }
    void setMinimapVisible(bool v) { m_minimapVisible = v; }
    void toggleMinimap() { m_minimapVisible = !m_minimapVisible; }

    float getDayTime() const { return m_dayTime; }
    void setDayTime(float t) { m_dayTime = t; }
    void advanceDayTime(float dt);
    float getDeathTimer() const { return m_deathTimer; }
    void setDeathTimer(float t) { m_deathTimer = t; }
    float& getDeathTimerRef() { return m_deathTimer; }

    int getCurrentSlot() const { return m_currentSlot; }
    WorldSize getWorldSize() const { return m_worldSize; }
    Difficulty getDifficulty() const { return m_difficulty; }
    void setDifficulty(Difficulty d) { m_difficulty = d; }
    const std::string& getWorldName() const { return m_worldName; }
    unsigned int getSeed() const { return m_seed; }

    std::mt19937& getRNG() { return m_rng; }
    bool isBossSummonRequested() const { return m_bossSummonRequested; }
    void requestBossSummon() { m_bossSummonRequested = true; }
    void clearBossSummon() { m_bossSummonRequested = false; }

private:
    std::unique_ptr<World> m_world;
    std::unique_ptr<Player> m_player;
    std::vector<std::unique_ptr<Mob>> m_mobs;
    std::vector<Projectile> m_projectiles;
    std::vector<TempHitbox> m_tempHitboxes;
    ParticleSystem m_particles;
    std::unique_ptr<Minimap> m_minimap;
    bool m_minimapVisible = true;

    float m_dayTime = 0.0f;
    float m_deathTimer = 0.0f;
    int m_currentSlot = -1;
    WorldSize m_worldSize = WorldSize::Medium;
    Difficulty m_difficulty = Difficulty::Normal;
    std::string m_worldName;
    unsigned int m_seed = 0;

    std::mt19937 m_rng;
    bool m_bossSummonRequested = false;
};
