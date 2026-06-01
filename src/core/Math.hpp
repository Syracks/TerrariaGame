#pragma once

#include "Constants.hpp"
#include <raylib.h>
#include <cmath>
#include <algorithm>

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
    return tileCoord & (constants::CHUNK_SIZE - 1);
}

inline Rectangle getScaledDestRect() {
    float sw = static_cast<float>(GetScreenWidth());
    float sh = static_cast<float>(GetScreenHeight());
    float scale = std::min(sw / static_cast<float>(constants::VIRTUAL_WIDTH),
                           sh / static_cast<float>(constants::VIRTUAL_HEIGHT));
    float w = static_cast<float>(constants::VIRTUAL_WIDTH) * scale;
    float h = static_cast<float>(constants::VIRTUAL_HEIGHT) * scale;
    return { (sw - w) * 0.5f, (sh - h) * 0.5f, w, h };
}

inline Vector2 getVirtualMouse() {
    Vector2 m = GetMousePosition();
    Rectangle dest = getScaledDestRect();
    if (dest.width <= 0.0f || dest.height <= 0.0f)
        return {0.0f, 0.0f};
    return {
        (m.x - dest.x) * static_cast<float>(constants::VIRTUAL_WIDTH) / dest.width,
        (m.y - dest.y) * static_cast<float>(constants::VIRTUAL_HEIGHT) / dest.height
    };
}

}
