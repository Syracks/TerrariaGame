#pragma once

#include <raylib.h>
#include "world/Tile.hpp"

class World;
class Player;

class MiningSystem {
public:
    static void tryMineTile(World& world, Player& player, int tileX, int tileY);
    static void tryPlace(World& world, Player& player, const Camera2D& camera);
    static float getMiningTime(TileId tile, TileId tool);
};
