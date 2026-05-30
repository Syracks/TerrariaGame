#pragma once

#include "world/Tile.hpp"
#include <memory>
#include <vector>
#include <random>

class Player;
class World;
class Mob;
class ParticleSystem;

class DeathSystem {
public:
    static bool updateDeath(Player& player, World& world,
                            std::vector<std::unique_ptr<Mob>>& mobs,
                            float& deathTimer, float dt,
                            ParticleSystem& particles,
                            std::mt19937& rng);

    static Vector2 findSafeSpawnPosition(const World& world);
};
