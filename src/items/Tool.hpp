#pragma once

#include "world/Tile.hpp"

class World;
class Inventory;

bool isTool(TileId id);
bool isTreeTile(TileId id);
bool isWallItem(TileId id);
void fellTree(World& world, int tileX, int tileY, Inventory& inventory);
