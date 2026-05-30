#pragma once

#include <raylib.h>
#include "world/Tile.hpp"

class World;
class Player;
class ParticleSystem;
class Minimap;

class InteractionSystem {
public:
    static void handleMinePress(Player& player, const World& world, const Camera2D& camera);
    static void handleSwingCompletion(Player& player, World& world,
                                       ParticleSystem& particles, Minimap& minimap);
    static void handlePlacePress(Player& player, World& world,
                                  const Camera2D& camera, Minimap& minimap);
    static void handleChestInteraction(Player& player, World& world,
                                        const Camera2D& camera);
};
