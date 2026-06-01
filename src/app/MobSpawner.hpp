#pragma once

#include "entities/Mob.hpp"
#include "core/Constants.hpp"
#include <memory>
#include <vector>
#include <random>

class World;
class Player;

class MobSpawner {
public:
    static void spawnSlimes(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                            const Player& player, std::mt19937& rng,
                            Difficulty difficulty = Difficulty::Normal);
    static void spawnZombies(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                             const Player& player, std::mt19937& rng,
                             Difficulty difficulty = Difficulty::Normal);
    static void updateNightSpawning(World& world, std::vector<std::unique_ptr<Mob>>& mobs,
                                    const Player& player, float dayTime, float dt,
                                    std::mt19937& rng,
                                    Difficulty difficulty = Difficulty::Normal);
};
