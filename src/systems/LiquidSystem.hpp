#pragma once

#include <cstdint>

class World;

class LiquidSystem {
public:
    static void update(World& world, float dt);
    static bool isInWater(const World& world, float worldX, float worldY, float width, float height);
    static bool isInLava(const World& world, float worldX, float worldY, float width, float height);
};
