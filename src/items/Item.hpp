#pragma once

#include "world/Tile.hpp"

// Deprecated: Use ItemStack instead (ItemStack.hpp)
struct Item {
    TileId tileId;
    int count;
};
