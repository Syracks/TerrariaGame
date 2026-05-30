#pragma once

#include "world/Tile.hpp"

class World;
class Inventory;

bool isTool(TileId id);
bool isPickaxe(TileId id);
bool isAxe(TileId id);
bool isSword_(TileId id);
bool isHammer(TileId id);
bool isTreeTile(TileId id);
bool isWallItem(TileId id);
void fellTree(World& world, int tileX, int tileY, Inventory& inventory);
int getToolMiningLevel(TileId id);
