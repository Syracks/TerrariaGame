#pragma once

#include <memory>
#include <vector>

class Player;
class Mob;
class ParticleSystem;

class CombatSystem {
public:
    static void checkSwordHit(Player& player,
                              std::vector<std::unique_ptr<Mob>>& mobs,
                              ParticleSystem& particles);
    static void checkMobContactDamage(const std::vector<std::unique_ptr<Mob>>& mobs,
                                      Player& player);
};
