#pragma once

namespace constants {

constexpr int VIRTUAL_WIDTH  = 1280;
constexpr int VIRTUAL_HEIGHT = 720;
constexpr int SCREEN_WIDTH = VIRTUAL_WIDTH;
constexpr int SCREEN_HEIGHT = VIRTUAL_HEIGHT;
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

constexpr int RESOLUTION_COUNT = 4;
inline const int RESOLUTIONS[RESOLUTION_COUNT][2] = {
    {1280, 720},
    {1600, 900},
    {1920, 1080},
    {2560, 1440}
};

}

enum class WorldSize { Small, Medium, Large };
enum class Difficulty { Normal, Hardcore };

struct WorldDimensions {
    int width;
    int height;
};

inline WorldDimensions getWorldDimensions(WorldSize size) {
    switch (size) {
        case WorldSize::Small:   return {1024, 256};
        case WorldSize::Medium:  return {2048, 400};
        case WorldSize::Large:   return {4096, 600};
        default:                 return {2048, 400};
    }
}

inline const char* worldSizeName(WorldSize size) {
    switch (size) {
        case WorldSize::Small:   return "Small";
        case WorldSize::Medium:  return "Medium";
        case WorldSize::Large:   return "Large";
        default:                 return "Medium";
    }
}

inline const char* difficultyName(Difficulty d) {
    switch (d) {
        case Difficulty::Normal:   return "Normal";
        case Difficulty::Hardcore: return "Hardcore";
        default:                   return "Normal";
    }
}
