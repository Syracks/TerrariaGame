#include "LiquidSystem.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/Tile.hpp"
#include "world/TileRegistry.hpp"
#include "core/Constants.hpp"

#include <cmath>
#include <vector>
#include <utility>
#include <algorithm>
#include <cstring>

namespace {
    constexpr int FLOW_RATE_WATER = 4;
    constexpr int FLOW_RATE_LAVA = 2;
    constexpr int MIN_LIQUID = 8;
    constexpr int MAX_LIQUID = MAX_LIQUID_LEVEL;
    constexpr float TICK_INTERVAL = 0.1f;
    constexpr int MAX_TILES_PER_TICK = 2000;
}

static bool isSolidBlock(const World& world, int x, int y) {
    if (!world.isInBounds(x, y)) return true;
    return world.isSolid(x, y);
}

void LiquidSystem::update(World& world, float dt) {
    static float accumulator = 0.0f;
    accumulator += dt;
    if (accumulator < TICK_INTERVAL) return;
    accumulator = 0.0f;

    static std::vector<Chunk*> liquidChunks;
    liquidChunks.clear();
    world.getChunksWithLiquid(liquidChunks);
    if (liquidChunks.empty()) return;

    static std::vector<std::pair<int,int>> changes;
    changes.clear();
    changes.reserve(MAX_TILES_PER_TICK);
    int processed = 0;

    auto absdiff = [](int a, int b) { return a > b ? a - b : b - a; };

    for (Chunk* chunk : liquidChunks) {
        int baseX = chunk->getChunkX() * constants::CHUNK_SIZE;
        int baseY = chunk->getChunkY() * constants::CHUNK_SIZE;

        for (int ly = constants::CHUNK_SIZE - 1; ly >= 0; --ly) {
            for (int lx = 0; lx < constants::CHUNK_SIZE; ++lx) {
                int x = baseX + lx;
                int y = baseY + ly;

                uint8_t water = chunk->getWater(lx, ly);
                uint8_t lava = chunk->getLava(lx, ly);
                if (water == 0 && lava == 0) continue;

                bool isLava = lava > 0 && lava >= water;
                uint8_t level = isLava ? lava : water;
                int fr = isLava ? FLOW_RATE_LAVA : FLOW_RATE_WATER;

                if (level < MIN_LIQUID) {
                    changes.push_back({x, y});
                    if (++processed >= MAX_TILES_PER_TICK) goto process;
                    continue;
                }

                int belowY = y + 1;
                if (belowY < constants::WORLD_HEIGHT && !isSolidBlock(world, x, belowY)) {
                    uint8_t belowLiquid = isLava ? world.getLava(x, belowY) : world.getWater(x, belowY);
                    if (belowLiquid < MAX_LIQUID) {
                        int transfer = std::min(fr, std::min((int)level, MAX_LIQUID - (int)belowLiquid));
                        if (transfer > 0) {
                            changes.push_back({x, y});
                            changes.push_back({x, belowY});
                            if (++processed >= MAX_TILES_PER_TICK) goto process;
                        }
                    }
                } else {
                    for (int dir : {-1, 1}) {
                        int nx = x + dir;
                        if (nx < 0 || nx >= constants::WORLD_WIDTH) continue;
                        if (isSolidBlock(world, nx, y)) continue;
                        uint8_t nl = isLava ? world.getLava(nx, y) : world.getWater(nx, y);
                        if (nl < level) {
                            int transfer = std::min(fr, (level - (int)nl) / 2);
                            if (transfer > 0) {
                                changes.push_back({x, y});
                                changes.push_back({nx, y});
                                if (++processed >= MAX_TILES_PER_TICK) goto process;
                            }
                        }
                    }
                }

                for (int d = 0; d < 4; ++d) {
                    static const int dx[] = {-1, 1, 0, 0};
                    static const int dy[] = {0, 0, -1, 1};
                    int nx = x + dx[d];
                    int ny = y + dy[d];
                    if (!world.isInBounds(nx, ny)) continue;
                    uint8_t nl = isLava ? world.getLava(nx, ny) : world.getWater(nx, ny);
                    if (absdiff(level, nl) > 1 && nl < MAX_LIQUID && level > 0) {
                        changes.push_back({x, y});
                        if (++processed >= MAX_TILES_PER_TICK) goto process;
                        break;
                    }
                }
            }
        }
    }

process:
    if (changes.empty()) return;

    std::sort(changes.begin(), changes.end());
    changes.erase(std::unique(changes.begin(), changes.end()), changes.end());

    if (changes.size() > MAX_TILES_PER_TICK)
        changes.resize(MAX_TILES_PER_TICK);

    for (auto& [tx, ty] : changes) {
        uint8_t water = world.getWater(tx, ty);
        uint8_t lava = world.getLava(tx, ty);
        if (water == 0 && lava == 0) continue;

        bool isLava = lava > 0 && lava >= water;
        uint8_t level = isLava ? lava : water;
        int fr = isLava ? FLOW_RATE_LAVA : FLOW_RATE_WATER;
        int remaining = level;

        if (level < MIN_LIQUID) {
            if (isLava) world.setLava(tx, ty, 0);
            else world.setWater(tx, ty, 0);
            continue;
        }

        int belowY = ty + 1;
        if (belowY < constants::WORLD_HEIGHT && !isSolidBlock(world, tx, belowY)) {
            uint8_t belowLiquid = isLava ? world.getLava(tx, belowY) : world.getWater(tx, belowY);
            if (belowLiquid < MAX_LIQUID) {
                int space = MAX_LIQUID - belowLiquid;
                int transfer = std::min(fr, std::min(remaining, space));
                if (transfer > 0) {
                    remaining -= transfer;
                    if (isLava) world.setLava(tx, belowY, belowLiquid + transfer);
                    else world.setWater(tx, belowY, belowLiquid + transfer);
                }
            }
        }

        if (remaining > 0 && isSolidBlock(world, tx, ty + 1)) {
            for (int dir : {-1, 1}) {
                int nx = tx + dir;
                if (nx < 0 || nx >= constants::WORLD_WIDTH) continue;
                if (isSolidBlock(world, nx, ty)) continue;
                uint8_t nl = isLava ? world.getLava(nx, ty) : world.getWater(nx, ty);
                if (nl < (uint8_t)remaining) {
                    int transfer = std::min(fr, (remaining - (int)nl) / 2);
                    if (transfer > 0) {
                        remaining -= transfer;
                        if (isLava) world.setLava(nx, ty, nl + transfer);
                        else world.setWater(nx, ty, nl + transfer);
                    }
                }
            }
        }

        if (isLava) world.setLava(tx, ty, remaining);
        else world.setWater(tx, ty, remaining);
    }
}

bool LiquidSystem::isInWater(const World& world, float worldX, float worldY, float width, float height) {
    int tx1 = (int)(worldX / constants::TILE_SIZE);
    int ty1 = (int)(worldY / constants::TILE_SIZE);
    int tx2 = (int)((worldX + width) / constants::TILE_SIZE);
    int ty2 = (int)((worldY + height) / constants::TILE_SIZE);

    for (int ty = ty1; ty <= ty2; ++ty) {
        for (int tx = tx1; tx <= tx2; ++tx) {
            if (world.isInBounds(tx, ty) && world.getWater(tx, ty) > MIN_LIQUID)
                return true;
        }
    }
    return false;
}

bool LiquidSystem::isInLava(const World& world, float worldX, float worldY, float width, float height) {
    int tx1 = (int)(worldX / constants::TILE_SIZE);
    int ty1 = (int)(worldY / constants::TILE_SIZE);
    int tx2 = (int)((worldX + width) / constants::TILE_SIZE);
    int ty2 = (int)((worldY + height) / constants::TILE_SIZE);

    for (int ty = ty1; ty <= ty2; ++ty) {
        for (int tx = tx1; tx <= tx2; ++tx) {
            if (world.isInBounds(tx, ty) && world.getLava(tx, ty) > MIN_LIQUID)
                return true;
        }
    }
    return false;
}
