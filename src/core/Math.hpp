#pragma once

#include "Constants.hpp"
#include <cmath>

namespace math {

inline int worldToTileX(float worldX) {
    return static_cast<int>(std::floor(worldX / constants::TILE_SIZE));
}

inline int worldToTileY(float worldY) {
    return static_cast<int>(std::floor(worldY / constants::TILE_SIZE));
}

inline float tileToWorldX(int tileX) {
    return static_cast<float>(tileX * constants::TILE_SIZE);
}

inline float tileToWorldY(int tileY) {
    return static_cast<float>(tileY * constants::TILE_SIZE);
}

inline int chunkFromTile(int tileCoord) {
    return static_cast<int>(std::floor(static_cast<float>(tileCoord) / constants::CHUNK_SIZE));
}

inline int localTileInChunk(int tileCoord) {
    return ((tileCoord % constants::CHUNK_SIZE) + constants::CHUNK_SIZE) % constants::CHUNK_SIZE;
}

}
