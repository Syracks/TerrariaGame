#pragma once

namespace constants {

constexpr int SCREEN_WIDTH = 1280;
constexpr int SCREEN_HEIGHT = 720;
constexpr int TILE_SIZE = 16;
constexpr int CHUNK_SIZE = 32;
inline int WORLD_WIDTH = 2048;
inline int WORLD_HEIGHT = 400;
constexpr float GRAVITY = 1200.0f;
constexpr float PLAYER_SPEED = 220.0f;
constexpr float JUMP_SPEED = -350.0f;
constexpr float PLAYER_REACH = 5.0f;
constexpr int HOTBAR_SLOTS = 9;
constexpr int INVENTORY_SLOTS = 45;
constexpr int TARGET_FPS = 60;

}

enum class WorldSize { Small, Medium, Large };

struct WorldDimensions {
    int width;
    int height;
};

inline WorldDimensions getWorldDimensions(WorldSize size) {
    switch (size) {
        case WorldSize::Small:   return {1024, 256};
        case WorldSize::Medium:  return {2048, 400};
        case WorldSize::Large:   return {4096, 600};
    }
    return {2048, 400};
}

inline const char* worldSizeName(WorldSize size) {
    switch (size) {
        case WorldSize::Small:   return "Small";
        case WorldSize::Medium:  return "Medium";
        case WorldSize::Large:   return "Large";
    }
    return "Medium";
}
