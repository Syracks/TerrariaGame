#pragma once

#include "world/Tile.hpp"

struct ItemStack {
    TileId tileId = TileId::Air;
    int count = 0;
};
