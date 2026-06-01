#pragma once

#include <cstdint>
#include <vector>
#include <utility>

class World;
class Chunk;

class LiquidSystem {
public:
    void update(World& world, float dt);

    static bool isInWater(const World& world, float worldX, float worldY, float width, float height);
    static bool isInLava(const World& world, float worldX, float worldY, float width, float height);

private:
    bool handleWaterLavaContact(World& world, int x, int y);

    float m_accumulator = 0.0f;
    std::vector<Chunk*> m_liquidChunks;
    std::vector<std::pair<int, int>> m_changes;
};
