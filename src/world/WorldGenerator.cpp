#include "WorldGenerator.hpp"
#include "World.hpp"
#include "Tile.hpp"
#include "core/Constants.hpp"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

namespace {
    constexpr int SURFACE_SAFE_DEPTH = 30;
    constexpr int SPAWN_SAFE_RADIUS = 90;
}

WorldGenerator::PRNG::PRNG(unsigned int seed)
    : state(seed) {}

int WorldGenerator::PRNG::range(int min, int max) {
    state = state * 1103515245u + 12345u;
    if (max <= min) return min;
    return min + (state / 65536u) % (max - min);
}

float WorldGenerator::PRNG::rangeF(float min, float max) {
    state = state * 1103515245u + 12345u;
    float t = static_cast<float>(state % 10000u) / 10000.0f;
    return min + t * (max - min);
}

WorldGenerator::PerlinNoise::PerlinNoise(unsigned int seed) {
    perm.resize(512);
    std::vector<int> p(256);
    for (int i = 0; i < 256; ++i) p[i] = i;

    std::mt19937 rng(seed);
    std::shuffle(p.begin(), p.end(), rng);
    for (int i = 0; i < 512; ++i) perm[i] = p[i & 255];
}

float WorldGenerator::PerlinNoise::fade(float t) const {
    return t * t * t * (t * (t * 6.0f - 15.0f) + 10.0f);
}

float WorldGenerator::PerlinNoise::lerp(float a, float b, float t) const {
    return a + t * (b - a);
}

float WorldGenerator::PerlinNoise::grad(int hash, float x, float y) const {
    int h = hash & 3;
    float u = (h & 1) ? -x : x;
    float v = (h & 2) ? -y : y;
    return (h & 1) ? v : u;
}

float WorldGenerator::PerlinNoise::noise(float x, float y) const {
    int xi = static_cast<int>(std::floor(x)) & 255;
    int yi = static_cast<int>(std::floor(y)) & 255;
    float xf = x - std::floor(x);
    float yf = y - std::floor(y);
    float u = fade(xf);
    float v = fade(yf);

    int aa = perm[perm[xi] + yi];
    int ab = perm[perm[xi] + yi + 1];
    int ba = perm[perm[xi + 1] + yi];
    int bb = perm[perm[xi + 1] + yi + 1];

    float x1 = lerp(grad(aa, xf, yf), grad(ba, xf - 1.0f, yf), u);
    float x2 = lerp(grad(ab, xf, yf - 1.0f), grad(bb, xf - 1.0f, yf - 1.0f), u);
    return lerp(x1, x2, v);
}

float WorldGenerator::PerlinNoise::octaveNoise(float x, float y, int octaves, float persistence, float scale) const {
    float total = 0.0f;
    float amplitude = 1.0f;
    float maxValue = 0.0f;
    float freq = scale;
    for (int i = 0; i < octaves; ++i) {
        total += noise(x * freq, y * freq) * amplitude;
        maxValue += amplitude;
        amplitude *= persistence;
        freq *= 2.0f;
    }
    return total / maxValue;
}

WorldGenerator::WorldGenerator(World& world, unsigned int seed)
    : m_world(world), m_seed(seed), m_noise(seed), m_rng(seed) {
}

Biome WorldGenerator::getBiome(int tileX) const {
    float frac = static_cast<float>(tileX) / static_cast<float>(constants::WORLD_WIDTH);

    float n = m_noise.octaveNoise(static_cast<float>(tileX) * 0.005f, 0.0f, 2, 0.5f, 1.0f) * 0.02f;
    frac += n;

    if (frac < 0.12f) return Biome::Snow;
    if (frac < 0.28f) return Biome::Desert;
    if (frac < 0.60f) {
        float m = m_noise.octaveNoise(static_cast<float>(tileX) * 0.015f, 100.0f, 3, 0.5f, 1.0f);
        if (m > 0.4f) return Biome::Forest;
        return Biome::Plains;
    }
    return Biome::Jungle;
}

static float baseHeightForBiome(Biome biome) {
    switch (biome) {
        case Biome::Forest: return 0.29f;
        case Biome::Desert: return 0.34f;
        case Biome::Snow:   return 0.30f;
        case Biome::Plains: return 0.32f;
        case Biome::Jungle: return 0.34f;
        case Biome::Ocean:  return 0.38f;
        case Biome::Beach:  return 0.32f;
        default:            return 0.32f;
    }
}

int WorldGenerator::getSurfaceHeight(int tileX) const {
    float nBig   = m_noise.octaveNoise(tileX * 0.008f,   0.0f, 4, 0.5f, 1.0f);
    float nMed   = m_noise.octaveNoise(tileX * 0.030f, 100.0f, 3, 0.5f, 1.0f);
    float nSmall = m_noise.octaveNoise(tileX * 0.080f, 200.0f, 2, 0.5f, 1.0f);

    Biome biome = getBiome(tileX);
    float baseFrac = baseHeightForBiome(biome);
    const float blendRadius = 30.0f;
    float blendedFrac = 0.0f;
    float totalWeight = 0.0f;
    for (int dx = -static_cast<int>(blendRadius); dx <= static_cast<int>(blendRadius); ++dx) {
        int tx = tileX + dx;
        if (tx < 0 || tx >= constants::WORLD_WIDTH) continue;
        float w = 1.0f - std::abs(static_cast<float>(dx)) / blendRadius;
        blendedFrac += baseHeightForBiome(getBiome(tx)) * w;
        totalWeight += w;
    }
    if (totalWeight > 0.0f) baseFrac = blendedFrac / totalWeight;

    float heightFrac = baseFrac + nBig * 0.030f + nMed * 0.020f + nSmall * 0.008f;
    heightFrac = std::clamp(heightFrac, 0.22f, 0.50f);

    return static_cast<int>(heightFrac * static_cast<float>(constants::WORLD_HEIGHT));
}

bool WorldGenerator::isSolidBelow(int tileX, int tileY) const {
    if (tileY + 1 >= constants::WORLD_HEIGHT) return true;
    return m_world.getTile(tileX, tileY + 1) != TileId::Air;
}

bool WorldGenerator::isReplaceableForCave(TileId id) const {
    return id == TileId::Dirt || id == TileId::Stone || id == TileId::Sand ||
           id == TileId::SnowBlock || id == TileId::Ice || id == TileId::Clay ||
           id == TileId::Gravel || id == TileId::Granite || id == TileId::Marble ||
           id == TileId::Mud;
}

void WorldGenerator::buildWorldMaps() {
    m_surfaceHeight.resize(constants::WORLD_WIDTH);
    m_biomeMap.resize(constants::WORLD_WIDTH);

    for (int x = 0; x < constants::WORLD_WIDTH; ++x) {
        m_biomeMap[x] = getBiome(x);
        m_surfaceHeight[x] = getSurfaceHeight(x);
    }

    for (int pass = 0; pass < 4; ++pass) {
        if (pass == 0) {
            for (int x = 1; x < constants::WORLD_WIDTH; ++x) {
                int diff = m_surfaceHeight[x] - m_surfaceHeight[x - 1];
                if (diff > 2) m_surfaceHeight[x] = m_surfaceHeight[x - 1] + 2;
                else if (diff < -2) m_surfaceHeight[x] = m_surfaceHeight[x - 1] - 2;
            }
        }

        std::vector<int> smoothed(constants::WORLD_WIDTH);
        for (int x = 0; x < constants::WORLD_WIDTH; ++x) {
            int sum = 0;
            int count = 0;
            int r = (pass < 2) ? 2 : 1;
            for (int dx = -r; dx <= r; ++dx) {
                int tx = x + dx;
                if (tx >= 0 && tx < constants::WORLD_WIDTH) {
                    sum += m_surfaceHeight[tx];
                    ++count;
                }
            }
            smoothed[x] = sum / count;
        }
        m_surfaceHeight = std::move(smoothed);
    }
}

bool WorldGenerator::isNearBiomeEdge(int x, int radius) const {
    Biome b = m_biomeMap[x];
    for (int dx = -radius; dx <= radius; ++dx) {
        int tx = x + dx;
        if (tx < 0 || tx >= constants::WORLD_WIDTH) continue;
        if (m_biomeMap[tx] != b) return true;
    }
    return false;
}

bool WorldGenerator::hasNearbyAir(int cx, int cy, int radius) const {
    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int tx = cx + dx;
            int ty = cy + dy;
            if (m_world.isInBounds(tx, ty) && m_world.getTile(tx, ty) == TileId::Air)
                return true;
        }
    }
    return false;
}

void WorldGenerator::carveCircle(int cx, int cy, int radius) {
    int spawnX = constants::WORLD_WIDTH / 2;

    for (int dy = -radius; dy <= radius; ++dy) {
        for (int dx = -radius; dx <= radius; ++dx) {
            int tx = cx + dx;
            int ty = cy + dy;

            if (!m_world.isInBounds(tx, ty)) {
                continue;
            }

            if (std::abs(tx - spawnX) < SPAWN_SAFE_RADIUS) {
                continue;
            }

            if (ty < m_surfaceHeight[tx] + SURFACE_SAFE_DEPTH) {
                continue;
            }

            float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
            if (dist > static_cast<float>(radius) + 0.5f) {
                continue;
            }

            TileId t = m_world.getTile(tx, ty);
            if (isReplaceableForCave(t)) {
                m_world.setTile(tx, ty, TileId::Air);
            }
        }
    }
}

void WorldGenerator::protectSpawnArea() {
    int centerX = constants::WORLD_WIDTH / 2;
    int halfWidth = 25;
    int flatHalf = 15;

    for (int x = centerX - halfWidth; x <= centerX + halfWidth; ++x) {
        if (x < 0 || x >= constants::WORLD_WIDTH) continue;
        m_biomeMap[x] = Biome::Forest;

        float edgeFactor = 0.0f;
        if (x < centerX - flatHalf)
            edgeFactor = static_cast<float>(centerX - flatHalf - x) / static_cast<float>(halfWidth - flatHalf);
        else if (x > centerX + flatHalf)
            edgeFactor = static_cast<float>(x - centerX - flatHalf) / static_cast<float>(halfWidth - flatHalf);

        int baseSurface = m_surfaceHeight[centerX];
        if (edgeFactor > 0.0f) {
            int edgeSurface = m_surfaceHeight[x];
            m_surfaceHeight[x] = baseSurface + static_cast<int>((edgeSurface - baseSurface) * edgeFactor);
        } else {
            m_surfaceHeight[x] = baseSurface;
        }
    }
}

void WorldGenerator::generateWormCaves() {
    int spawnX = constants::WORLD_WIDTH / 2;

    int wormCount = std::max(1, constants::WORLD_WIDTH / 120);

    for (int w = 0; w < wormCount; ++w) {
        int startX = m_rng.range(10, constants::WORLD_WIDTH - 10);

        if (std::abs(startX - spawnX) < SPAWN_SAFE_RADIUS) {
            continue;
        }

        int surfaceY = m_surfaceHeight[startX];

        int startY = surfaceY + SURFACE_SAFE_DEPTH + 40 + m_rng.range(0, 80);

        if (startY >= constants::WORLD_HEIGHT - 25) {
            startY = constants::WORLD_HEIGHT - 25;
        }

        int x = startX;
        int y = startY;

        int radius = 2 + m_rng.range(0, 2);
        int steps = 35 + m_rng.range(0, 65);
        float angle = m_rng.rangeF(0.0f, 6.28f);

        for (int s = 0; s < steps; ++s) {
            carveCircle(x, y, radius);

            angle += m_rng.rangeF(-0.28f, 0.28f);

            x += static_cast<int>(std::cos(angle) * 2.0f);
            y += static_cast<int>(std::sin(angle) * 1.6f);

            if (x < 5 || x >= constants::WORLD_WIDTH - 5) break;
            if (y < m_surfaceHeight[x] + SURFACE_SAFE_DEPTH) break;
            if (y >= constants::WORLD_HEIGHT - 8) break;

            if (s % 24 == 0) {
                radius += m_rng.range(-1, 2);
                if (radius < 2) radius = 2;
                if (radius > 4) radius = 4;
            }
        }
    }
}

void WorldGenerator::rebuildSpawnArea() {
    int centerX = constants::WORLD_WIDTH / 2;
    int surfaceY = m_surfaceHeight[centerX];

    int halfWidth = 35;
    int clearHeight = 28;
    int dirtDepth = 10;
    int stoneDepth = 35;

    for (int x = centerX - halfWidth; x <= centerX + halfWidth; ++x) {
        if (x < 0 || x >= constants::WORLD_WIDTH) continue;

        int dx = std::abs(x - centerX);

        int y = surfaceY;

        if (dx > 18) {
            y += (dx - 18) / 5;
        }

        if (y < 10) y = 10;
        if (y > constants::WORLD_HEIGHT - 50) {
            y = constants::WORLD_HEIGHT - 50;
        }

        m_surfaceHeight[x] = y;
        m_biomeMap[x] = Biome::Forest;

        for (int ty = std::max(0, y - clearHeight); ty < y; ++ty) {
            m_world.setTile(x, ty, TileId::Air);
            m_world.setWall(x, ty, TileId::Air);
        }

        m_world.setTile(x, y, TileId::Grass);
        m_world.setWall(x, y, TileId::Air);

        for (int ty = y + 1; ty <= y + dirtDepth && ty < constants::WORLD_HEIGHT; ++ty) {
            m_world.setTile(x, ty, TileId::Dirt);
            m_world.setWall(x, ty, TileId::Air);
        }

        for (int ty = y + dirtDepth + 1;
             ty <= y + stoneDepth && ty < constants::WORLD_HEIGHT;
             ++ty) {
            m_world.setTile(x, ty, TileId::Stone);
            m_world.setWall(x, ty, TileId::Air);
        }
    }
}

void WorldGenerator::repairSurfaceLayer() {
    for (int x = 0; x < constants::WORLD_WIDTH; ++x) {
        if (x < 2 || x >= constants::WORLD_WIDTH - 2) {
            continue;
        }

        int surfaceY = m_surfaceHeight[x];

        if (surfaceY < 5 || surfaceY >= constants::WORLD_HEIGHT - 10) {
            continue;
        }

        Biome biome = m_biomeMap[x];

        TileId surfaceTile;
        TileId upperFill;
        TileId lowerFill;

        switch (biome) {
            case Biome::Snow:
                surfaceTile = TileId::SnowBlock;
                upperFill = TileId::SnowBlock;
                lowerFill = TileId::Stone;
                break;

            case Biome::Desert:
                surfaceTile = TileId::Sand;
                upperFill = TileId::Sand;
                lowerFill = TileId::Sand;
                break;

            case Biome::Jungle:
                surfaceTile = TileId::JungleGrass;
                upperFill = TileId::Mud;
                lowerFill = TileId::Mud;
                break;
            case Biome::Ocean:
                continue;
            case Biome::Beach:
                surfaceTile = TileId::Sand;
                upperFill = TileId::Sand;
                lowerFill = TileId::Sand;
                break;

            default:
                surfaceTile = TileId::Grass;
                upperFill = TileId::Dirt;
                lowerFill = TileId::Stone;
                break;
        }

        for (int y = surfaceY - 5; y < surfaceY; ++y) {
            if (y >= 0) {
                m_world.setTile(x, y, TileId::Air);
                m_world.setWall(x, y, TileId::Air);
            }
        }

        m_world.setTile(x, surfaceY, surfaceTile);

        for (int y = surfaceY + 1; y <= surfaceY + 8 && y < constants::WORLD_HEIGHT; ++y) {
            m_world.setTile(x, y, upperFill);
        }

        for (int y = surfaceY + 9; y <= surfaceY + 18 && y < constants::WORLD_HEIGHT; ++y) {
            if (m_world.getTile(x, y) == TileId::Air) {
                m_world.setTile(x, y, lowerFill);
            }
        }
    }
}

void WorldGenerator::generate(ProgressCallback progress) {
    buildWorldMaps();
    if (progress) progress(0.05f);

    protectSpawnArea();
    m_world.setBiomeData(m_biomeMap, m_surfaceHeight);
    if (progress) progress(0.10f);

    generateTerrain();
    if (progress) progress(0.20f);

    generateOceans();
    repairOceanTransitions();
    repairOceanSurface();
    m_world.setBiomeData(m_biomeMap, m_surfaceHeight);
    if (progress) progress(0.30f);

    repairSurfaceLayer();
    if (progress) progress(0.38f);

    generateCaves();
    if (progress) progress(0.48f);

    generateWormCaves();
    if (progress) progress(0.54f);

    generateUndergroundCabins();
    if (progress) progress(0.62f);

    generateOreVeins();
    if (progress) progress(0.70f);

    generateUndergroundPockets();
    if (progress) progress(0.77f);

    generateWaterPools();
    if (progress) progress(0.83f);

    generateLavaPools();
    if (progress) progress(0.88f);

    repairSurfaceLayer();
    if (progress) progress(0.93f);

    generateTrees();
    if (progress) progress(0.96f);

    postProcess();
    if (progress) progress(1.0f);
}

void WorldGenerator::generateTerrain() {
    const int underworldTop = constants::WORLD_HEIGHT - 25;

    for (int tileX = 0; tileX < constants::WORLD_WIDTH; ++tileX) {
        Biome biome = m_biomeMap[tileX];
        int surfaceY = m_surfaceHeight[tileX];

        for (int tileY = 0; tileY < constants::WORLD_HEIGHT; ++tileY) {
            if (tileY < surfaceY) {
                m_world.setTile(tileX, tileY, TileId::Air);
                continue;
            }
            if (tileY >= underworldTop) {
                m_world.setTile(tileX, tileY, TileId::Hellstone);
                continue;
            }

            int depth = tileY - surfaceY;

            if (tileY == surfaceY) {
                TileId fill;
                switch (biome) {
                    case Biome::Snow:     fill = TileId::SnowBlock; break;
                    case Biome::Desert:   fill = TileId::Sand; break;
                    case Biome::Jungle:   fill = TileId::JungleGrass; break;
                    case Biome::Ocean:
                    case Biome::Beach:    fill = TileId::Sand; break;
                    default:              fill = TileId::Grass; break;
                }
                if (isNearBiomeEdge(tileX, 6) && m_rng.range(0, 4) == 0) {
                    for (int dx = -6; dx <= 6; ++dx) {
                        int tx = tileX + dx;
                        if (tx >= 0 && tx < constants::WORLD_WIDTH && m_biomeMap[tx] != biome) {
                            switch (m_biomeMap[tx]) {
                                case Biome::Desert:   fill = TileId::Sand; break;
                                case Biome::Snow:     fill = TileId::SnowBlock; break;
                                case Biome::Jungle:   fill = TileId::Mud; break;
                                default:              fill = TileId::Dirt; break;
                            }
                            break;
                        }
                    }
                }
                m_world.setTile(tileX, tileY, fill);
                continue;
            }

            TileId fill;
            float n = m_noise.octaveNoise(tileX * 0.020f, tileY * 0.025f, 2, 0.5f, 1.0f);

            if (depth <= 3) {
                if (biome == Biome::Desert || biome == Biome::Ocean || biome == Biome::Beach)
                    fill = TileId::Sand;
                else if (biome == Biome::Snow)   fill = TileId::SnowBlock;
                else if (biome == Biome::Jungle) fill = TileId::Mud;
                else                             fill = TileId::Dirt;
            } else if (depth <= 35) {
                float t = static_cast<float>(depth - 3) / 32.0f;
                t = std::clamp(t + n * 0.15f, 0.0f, 1.0f);
                if (t < 0.55f) {
                    if (biome == Biome::Desert || biome == Biome::Ocean || biome == Biome::Beach)
                        fill = TileId::Sand;
                    else if (biome == Biome::Jungle) fill = TileId::Mud;
                    else if (biome == Biome::Snow && depth < 6) fill = TileId::SnowBlock;
                    else                             fill = TileId::Dirt;
                } else {
                    fill = TileId::Stone;
                }
            } else {
                fill = TileId::Stone;
                if (n > 0.35f && depth > 40) {
                    fill = (n > 0.55f) ? TileId::Marble : TileId::Granite;
                }
            }

            m_world.setTile(tileX, tileY, fill);
        }

        if (biome == Biome::Snow) {
            int cover = m_rng.range(1, 4);
            for (int y = surfaceY - cover; y < surfaceY; ++y) {
                if (y >= 0 && m_world.getTile(tileX, y) == TileId::Air) {
                    m_world.setTile(tileX, y, TileId::SnowBlock);
                }
            }
        }
    }
}

void WorldGenerator::generateCaves() {
    int spawnX = constants::WORLD_WIDTH / 2;

    for (int tileX = 0; tileX < constants::WORLD_WIDTH; ++tileX) {
        if (std::abs(tileX - spawnX) < SPAWN_SAFE_RADIUS) continue;

        Biome biome = m_biomeMap[tileX];
        int surfaceY = m_surfaceHeight[tileX];
        int caveStartY = surfaceY + SURFACE_SAFE_DEPTH;
        if (caveStartY < 6) caveStartY = 6;

        for (int tileY = caveStartY; tileY < constants::WORLD_HEIGHT - 5; ++tileY) {
            TileId tile = m_world.getTile(tileX, tileY);
            if (!isReplaceableForCave(tile)) continue;

            int depth = tileY - surfaceY;
            float caveNoise = m_noise.octaveNoise(
                tileX * 0.030f, tileY * 0.030f, 3, 0.5f, 1.0f);
            float cavernNoise = m_noise.octaveNoise(
                tileX * 0.012f, tileY * 0.012f, 2, 0.5f, 1.0f);
            float smallNoise = m_noise.octaveNoise(
                tileX * 0.060f, tileY * 0.060f, 2, 0.5f, 1.0f);

            float threshold;
            float combined = caveNoise;

            if (depth < 50) {

                threshold = 0.48f - static_cast<float>(depth - SURFACE_SAFE_DEPTH) / 150.0f;
                combined += std::max(0.0f, smallNoise - 0.35f) * 0.10f;
            } else if (depth < 80) {

                threshold = 0.42f - static_cast<float>(depth - 50) / 200.0f;
                combined += std::max(0.0f, cavernNoise - 0.50f) * 0.15f;
            } else {

                threshold = 0.36f - static_cast<float>(depth - 80) / 300.0f;
                combined += std::max(0.0f, cavernNoise - 0.40f) * 0.25f;
            }

            float biomeBonus = (biome == Biome::Jungle) ? 0.05f : 0.0f;
            combined += biomeBonus;

            if (combined > threshold) {
                m_world.setTile(tileX, tileY, TileId::Air);
            }
        }
    }
}

void WorldGenerator::generateOreVeins() {
    int veinCount = constants::WORLD_WIDTH * constants::WORLD_HEIGHT / 600;

    for (int i = 0; i < veinCount; ++i) {
        int cx = m_rng.range(0, constants::WORLD_WIDTH);
        int cy = 40 + m_rng.range(0, constants::WORLD_HEIGHT - 90);
        int surfaceY = m_surfaceHeight[cx];
        int depth = cy - surfaceY;

        if (depth < 12) continue;

        TileId oreType;
        int maxSize;

        if (depth < 35) {
            int r = m_rng.range(0, 100);
            if (r < 75)      { oreType = TileId::CopperOre; maxSize = 8; }
            else if (r < 95) { oreType = TileId::IronOre;   maxSize = 5; }
            else             { oreType = TileId::CopperOre; maxSize = 4; }
        } else if (depth < 60) {
            int r = m_rng.range(0, 100);
            if (r < 30)      { oreType = TileId::CopperOre; maxSize = 6; }
            else if (r < 75) { oreType = TileId::IronOre;   maxSize = 7; }
            else if (r < 92) { oreType = TileId::GoldOre;   maxSize = 5; }
            else             { oreType = TileId::IronOre;   maxSize = 4; }
        } else if (depth < 90) {
            int r = m_rng.range(0, 100);
            if (r < 25)      { oreType = TileId::IronOre;   maxSize = 6; }
            else if (r < 70) { oreType = TileId::GoldOre;   maxSize = 6; }
            else             { oreType = TileId::GoldOre;   maxSize = 4; }
        } else {
            int r = m_rng.range(0, 100);
            if (r < 40)      { oreType = TileId::GoldOre;   maxSize = 7; }
            else if (r < 70) { oreType = TileId::IronOre;   maxSize = 5; }
            else             { oreType = TileId::GoldOre;   maxSize = 4; }
        }

        int size = 4 + m_rng.range(0, maxSize);
        int veinW = 2 + m_rng.range(0, size / 2);
        int veinH = 2 + m_rng.range(0, size / 2);

        for (int dy = -veinH; dy <= veinH; ++dy) {
            for (int dx = -veinW; dx <= veinW; ++dx) {
                float rx = static_cast<float>(dx) / static_cast<float>(veinW);
                float ry = static_cast<float>(dy) / static_cast<float>(veinH);
                float ellipse = rx * rx + ry * ry;
                float noiseVal = m_noise.noise(
                    static_cast<float>(cx + dx) * 0.1f,
                    static_cast<float>(cy + dy) * 0.1f);
                float threshold = 0.7f + noiseVal * 0.3f;
                if (ellipse < threshold && m_rng.range(0, 3) != 0) {
                    int tx = cx + dx;
                    int ty = cy + dy;
                    if (m_world.isInBounds(tx, ty)) {
                        TileId t = m_world.getTile(tx, ty);
                        if (t == TileId::Stone || t == TileId::Mud) {
                            m_world.setTile(tx, ty, oreType);
                        }
                    }
                }
            }
        }
    }
}

static bool canGrowTreeOn(TileId surface) {
    return surface == TileId::Grass || surface == TileId::JungleGrass || surface == TileId::Sand;
}

void WorldGenerator::generateTrees() {
    int spawnX = constants::WORLD_WIDTH / 2;
    int oceanMargin = 62;
    int lastTreeX = -9999;

    for (int tileX = 3; tileX < constants::WORLD_WIDTH - 3; ++tileX) {
        if (tileX < oceanMargin || tileX >= constants::WORLD_WIDTH - oceanMargin) continue;
        Biome biome = m_biomeMap[tileX];
        int surfaceY = m_surfaceHeight[tileX];

        if (std::abs(tileX - spawnX) < 12) continue;

        if (tileX - lastTreeX < 10) continue;

        TileId surfaceTile = m_world.getTile(tileX, surfaceY);
        if (!canGrowTreeOn(surfaceTile)) continue;
        if (m_world.getTile(tileX, surfaceY - 1) != TileId::Air) continue;


        int leftY  = (tileX > 0) ? m_surfaceHeight[tileX - 1] : surfaceY;
        int rightY = (tileX < constants::WORLD_WIDTH - 1) ? m_surfaceHeight[tileX + 1] : surfaceY;
        if (std::abs(surfaceY - leftY) > 1 || std::abs(surfaceY - rightY) > 1) continue;
        if (biome == Biome::Forest) {
            if (m_rng.range(0, 7) != 0) continue;
        } else if (biome == Biome::Plains) {
            if (m_rng.range(0, 18) != 0) continue;
        } else if (biome == Biome::Jungle) {
            if (m_rng.range(0, 6) != 0) continue;
        } else if (biome == Biome::Desert && surfaceTile == TileId::Sand) {
            if (m_rng.range(0, 5) != 0) continue;
        } else if (biome == Biome::Snow) {
            if (m_rng.range(0, 10) != 0) continue;
        } else {
            continue;
        }
        if (biome == Biome::Desert) {
            int height = 2 + m_rng.range(0, 3);
            bool blocked = false;
            for (int dx = -3; dx <= 3; ++dx) {
                int tx = tileX + dx;
                if (tx < 0 || tx >= constants::WORLD_WIDTH) continue;
                for (int dy = 1; dy <= 6; ++dy) {
                    int y = surfaceY - dy;
                    if (y >= 0 && m_world.getTile(tx, y) == TileId::Cactus) {
                        blocked = true;
                        break;
                    }
                }
                if (blocked) break;
            }
            if (blocked) continue;
            for (int dy = 1; dy <= height + 1; ++dy) {
                int y = surfaceY - dy;
                if (y < 0 || m_world.getTile(tileX, y) != TileId::Air) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) continue;

            for (int dy = 1; dy <= height; ++dy) {
                m_world.setTile(tileX, surfaceY - dy, TileId::Cactus);
            }
            lastTreeX = tileX;
            continue;
        }
        if (biome == Biome::Jungle) {
            int trunkHeight = 9 + m_rng.range(0, 5);
            int canopyRadius = 3 + m_rng.range(0, 2);
            bool blocked = false;
            for (int dy = 1; dy <= trunkHeight + canopyRadius + 1; ++dy) {
                int y = surfaceY - dy;
                if (y < 0 || m_world.getTile(tileX, y) != TileId::Air) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) continue;
            for (int dx = -canopyRadius - 1; dx <= canopyRadius + 1; ++dx) {
                for (int dy = -canopyRadius - 1; dy <= 1; ++dy) {
                    int lx = tileX + dx;
                    int ly = surfaceY - trunkHeight + dy;
                    if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                        if (m_world.getTile(lx, ly) != TileId::Air) {
                            blocked = true;
                            break;
                        }
                    }
                }
                if (blocked) break;
            }
            if (blocked) continue;
            for (int dy = 1; dy <= trunkHeight; ++dy) {
                m_world.setTile(tileX, surfaceY - dy, TileId::Wood);
            }
            for (int dy = -canopyRadius; dy <= 0; ++dy) {
                int r = (dy == 0) ? canopyRadius : canopyRadius - 1;
                for (int dx = -r; dx <= r; ++dx) {
                    if (dx == 0 && dy >= 0) continue;
                    int lx = tileX + dx;
                    int ly = surfaceY - trunkHeight + dy;
                    if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                        if (m_world.getTile(lx, ly) == TileId::Air) {
                            if (std::abs(dx) == r && std::abs(dy) == r) {
                                if (m_rng.range(0, 2) == 0) continue;
                            }
                            m_world.setTile(lx, ly, TileId::Leaf);
                        }
                    }
                }
            }
            lastTreeX = tileX;
            continue;
        }
        int trunkHeight = 7 + m_rng.range(0, 4);
        bool blocked = false;
        for (int dy = 1; dy <= trunkHeight + 4; ++dy) {
            int y = surfaceY - dy;
            if (y < 0 || m_world.getTile(tileX, y) != TileId::Air) {
                blocked = true;
                break;
            }
        }
        if (blocked) continue;
        for (int dx = -3; dx <= 3; ++dx) {
            for (int dy = -3; dy <= 2; ++dy) {
                int lx = tileX + dx;
                int ly = surfaceY - trunkHeight + dy;
                if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                    if (m_world.getTile(lx, ly) != TileId::Air) {
                        blocked = true;
                        break;
                    }
                }
            }
            if (blocked) break;
        }
        if (blocked) continue;

        for (int dy = 1; dy <= trunkHeight; ++dy) {
            m_world.setTile(tileX, surfaceY - dy, TileId::Wood);
        }
        for (int dy = -2; dy <= 1; ++dy) {
            int radius = (dy == 1) ? 1 : 2;
            for (int dx = -radius; dx <= radius; ++dx) {
                if (dx == 0 && dy >= 0 && dy <= 1) continue;
                int lx = tileX + dx;
                int ly = surfaceY - trunkHeight + dy;
                if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                    if (m_world.getTile(lx, ly) == TileId::Air) {
                        if (std::abs(dx) == radius && std::abs(dy) == 2) {
                            if (m_rng.range(0, 2) == 0) continue;
                        }
                        m_world.setTile(lx, ly, TileId::Leaf);
                    }
                }
            }
        }
        lastTreeX = tileX;
    }
}

void WorldGenerator::postProcess() {
    int spawnX = constants::WORLD_WIDTH / 2;

    for (int tileX = 0; tileX < constants::WORLD_WIDTH; ++tileX) {
        int surfaceY = m_surfaceHeight[tileX];

        for (int tileY = surfaceY; tileY < constants::WORLD_HEIGHT; ++tileY) {
            TileId tile = m_world.getTile(tileX, tileY);

            if (!isReplaceableForCave(tile)) continue;


            if (tile == TileId::Air && tileY > surfaceY + 1) {
                int solidCount = 0;
                for (int dy = -1; dy <= 1; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        if (dx == 0 && dy == 0) continue;
                        int tx = tileX + dx;
                        int ty = tileY + dy;
                        if (tx >= 0 && tx < constants::WORLD_WIDTH && ty >= 0 && ty < constants::WORLD_HEIGHT) {
                            TileId neighbor = m_world.getTile(tx, ty);
                            if (neighbor != TileId::Air) ++solidCount;
                        }
                    }
                }
                if (solidCount >= 7) {
                    m_world.setTile(tileX, tileY,
                        m_world.getTile(tileX, tileY + 1) != TileId::Air
                            ? m_world.getTile(tileX, tileY - 1)
                            : TileId::Dirt);
                }
                continue;
            }


            if (tile != TileId::Air && tileY < constants::WORLD_HEIGHT - 1) {
                bool hasSupport = false;
                for (int dy = 1; dy <= 2 && tileY + dy < constants::WORLD_HEIGHT; ++dy) {
                    TileId below = m_world.getTile(tileX, tileY + dy);
                    if (below != TileId::Air) { hasSupport = true; break; }
                }
                if (!hasSupport) {
                    m_world.setTile(tileX, tileY, TileId::Air);
                }
            }
        }
    }


    for (int x = spawnX - 15; x <= spawnX + 15; ++x) {
        if (x < 0 || x >= constants::WORLD_WIDTH) continue;
        for (int y = m_surfaceHeight[x] - 5; y < m_surfaceHeight[x]; ++y) {
            if (y >= 0) {
                TileId t = m_world.getTile(x, y);
                if (t != TileId::Air && t != TileId::Wood && t != TileId::Leaf) {
                    m_world.setTile(x, y, TileId::Air);
                }
            }
        }
    }
}

void WorldGenerator::generateUndergroundPockets() {
    int pocketCount = constants::WORLD_WIDTH / 4;

    for (int i = 0; i < pocketCount; ++i) {
        int cx = m_rng.range(0, constants::WORLD_WIDTH);
        Biome biome = m_biomeMap[cx];
        int surfaceY = m_surfaceHeight[cx];
        int cy = surfaceY + 15 + m_rng.range(0, constants::WORLD_HEIGHT - surfaceY - 30);
        if (cy >= constants::WORLD_HEIGHT - 30) continue;

        int depth = cy - surfaceY;
        TileId pocketType;

        if (depth < 20) {
            if (biome == Biome::Desert) pocketType = TileId::Sand;
            else pocketType = (m_rng.range(0, 3) == 0) ? TileId::Clay : TileId::Dirt;
        } else if (depth < 50) {
            pocketType = (m_rng.range(0, 2) == 0) ? TileId::Clay : TileId::Gravel;
        } else {
            int r = m_rng.range(0, 100);
            if (r < 30)       pocketType = TileId::Gravel;
            else if (r < 55)  pocketType = TileId::Granite;
            else if (r < 75)  pocketType = TileId::Marble;
            else              pocketType = TileId::Clay;
        }

        TileId centerTile = m_world.getTile(cx, cy);
        if (centerTile != TileId::Stone && centerTile != TileId::Dirt &&
            centerTile != TileId::Mud)
            continue;

        int radius = 2 + m_rng.range(0, 4);
        for (int dy = -radius; dy <= radius; ++dy) {
            for (int dx = -radius; dx <= radius; ++dx) {
                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                if (dist > radius) continue;
                if (dist > radius - 1 && m_rng.range(0, 3) == 0) continue;
                int tx = cx + dx;
                int ty = cy + dy;
                if (!m_world.isInBounds(tx, ty)) continue;
                TileId t = m_world.getTile(tx, ty);
                if (t == TileId::Stone || t == TileId::Dirt || t == TileId::Mud) {
                    m_world.setTile(tx, ty, pocketType);
                }
            }
        }
    }
}

void WorldGenerator::generateUndergroundCabins() {
    int cabinCount = 2 + m_rng.range(0, 3);
    int placed = 0;

    for (int attempt = 0; attempt < cabinCount * 20 && placed < cabinCount; ++attempt) {
        int cx = m_rng.range(30, constants::WORLD_WIDTH - 30);
        Biome biome = m_biomeMap[cx];

        if (biome == Biome::Desert || biome == Biome::Snow)
            continue;

        int surfaceY = m_surfaceHeight[cx];
        int cy = surfaceY + 20 + m_rng.range(0, 50);
        if (cy >= constants::WORLD_HEIGHT - 30) continue;

        if (!hasNearbyAir(cx, cy, 12)) continue;

        int roomW = 5 + m_rng.range(0, 3);
        int roomH = 4 + m_rng.range(0, 2);
        bool valid = true;
        for (int dx = -1; dx <= roomW; ++dx) {
            for (int dy = -1; dy <= roomH; ++dy) {
                int tx = cx + dx;
                int ty = cy + dy;
                if (!m_world.isInBounds(tx, ty)) { valid = false; break; }
                TileId t = m_world.getTile(tx, ty);
                if (t != TileId::Dirt && t != TileId::Stone && t != TileId::Mud &&
                    t != TileId::Clay && t != TileId::Gravel && t != TileId::Sand &&
                    t != TileId::Granite && t != TileId::Marble && t != TileId::Mud) {
                    valid = false; break;
                }
            }
            if (!valid) break;
        }
        if (!valid) continue;
        for (int dy = 0; dy < roomH; ++dy) {
            for (int dx = 0; dx < roomW; ++dx) {
                int tx = cx + dx;
                int ty = cy + dy;

                if (dx == 0 || dx == roomW - 1 || dy == 0 || dy == roomH - 1) {
                    m_world.setTile(tx, ty, TileId::Wood);
                } else {
                    m_world.setTile(tx, ty, TileId::Air);
                }
            }
        }
        int doorSide = m_rng.range(0, 3);
        int doorX, doorDir;
        if (doorSide == 0) { doorX = cx; doorDir = -1; }
        else if (doorSide == 1) { doorX = cx + roomW - 1; doorDir = 1; }
        else { doorX = cx + roomW / 2; doorDir = 0; }
        if (doorSide < 2) {
            int doorY = cy + roomH - 2;
            m_world.setTile(doorX, doorY, TileId::Air);
            int tunnelLen = 2 + m_rng.range(0, 3);
            for (int t = 1; t <= tunnelLen; ++t) {
                int tx = doorX + doorDir * t;
                if (m_world.isInBounds(tx, doorY)) {
                    TileId tile = m_world.getTile(tx, doorY);
                    if (tile != TileId::Air) {
                        m_world.setTile(tx, doorY, TileId::Air);
                    }
                }
            }
        } else {
            int doorY = cy - 1;
            if (m_world.isInBounds(doorX, doorY)) {
                TileId tile = m_world.getTile(doorX, doorY);
                if (tile != TileId::Air) {
                    m_world.setTile(doorX, doorY, TileId::Air);
                }
            }
        }

        placed++;
    }
}

void WorldGenerator::generateOceans() {
    int oceanWidth = 72;

    for (int side = 0; side < 2; ++side) {
        for (int x = 0; x < oceanWidth; ++x) {
            int tileX = (side == 0) ? x : constants::WORLD_WIDTH - 1 - x;
            if (tileX < 0 || tileX >= constants::WORLD_WIDTH) continue;

            int baseSurface = m_surfaceHeight[tileX];
            float frac = static_cast<float>(x) / static_cast<float>(oceanWidth);
            float drop = (1.0f - frac) * (1.0f - frac);

            int surfaceY = std::clamp(baseSurface, 5, constants::WORLD_HEIGHT - 10);
            m_surfaceHeight[tileX] = surfaceY;

            m_biomeMap[tileX] = (x < 45) ? Biome::Ocean : Biome::Beach;

            for (int y = 0; y < surfaceY; ++y) {
                if (m_world.isInBounds(tileX, y)) {
                    m_world.setTile(tileX, y, TileId::Air);
                    m_world.setWall(tileX, y, TileId::Air);
                }
            }


            if (x < 45) {
                int oceanFloor = surfaceY + 18 + static_cast<int>(drop * 12.0f);
                oceanFloor = std::min(oceanFloor, constants::WORLD_HEIGHT - 5);
                for (int y = surfaceY; y <= oceanFloor && y < constants::WORLD_HEIGHT; ++y) {
                    m_world.setTile(tileX, y, TileId::Air);
                    m_world.setWater(tileX, y, MAX_LIQUID_LEVEL);
                }
                for (int y = oceanFloor + 1; y <= oceanFloor + 4 && y < constants::WORLD_HEIGHT; ++y) {
                    m_world.setTile(tileX, y, TileId::Sand);
                }
            } else {
                int beachFloor = surfaceY + 6 + static_cast<int>(drop * 6.0f);
                beachFloor = std::min(beachFloor, constants::WORLD_HEIGHT - 5);
                for (int y = surfaceY; y <= beachFloor && y < constants::WORLD_HEIGHT; ++y) {
                    m_world.setTile(tileX, y, TileId::Sand);
                }
            }
        }
    }
}

void WorldGenerator::repairOceanTransitions() {
    int oceanWidth = 72;

    for (int side = 0; side < 2; ++side) {
        for (int x = 0; x < oceanWidth; ++x) {
            int tileX = (side == 0) ? x : constants::WORLD_WIDTH - 1 - x;
            if (tileX < 0 || tileX >= constants::WORLD_WIDTH) continue;

            int surfaceY = m_surfaceHeight[tileX];
            int nextTileX = (side == 0) ? tileX + 1 : tileX - 1;
            if (nextTileX < 0 || nextTileX >= constants::WORLD_WIDTH) continue;

            int nextSurfaceY = m_surfaceHeight[nextTileX];
            int diff = nextSurfaceY - surfaceY;

            if (std::abs(diff) > 3) {
                m_surfaceHeight[tileX] = surfaceY + diff / 2;
            }

            TileId surfaceTile = m_world.getTile(tileX, surfaceY);
            if (surfaceTile != TileId::Sand && surfaceTile != TileId::Air) {
                m_world.setTile(tileX, surfaceY, TileId::Sand);
                m_world.setWater(tileX, surfaceY, 0);
                m_world.setLava(tileX, surfaceY, 0);
                for (int y = surfaceY + 1; y <= surfaceY + 4 && y < constants::WORLD_HEIGHT; ++y) {
                    TileId t = m_world.getTile(tileX, y);
                    if (t == TileId::Air) {
                        m_world.setTile(tileX, y, TileId::Sand);
                        m_world.setWater(tileX, y, 0);
                        m_world.setLava(tileX, y, 0);
                    }
                }
            }
        }

        for (int pass = 0; pass < 2; ++pass) {
            std::vector<int> smoothed(constants::WORLD_WIDTH);
            for (int x = 0; x < oceanWidth; ++x) {
                int tileX = (side == 0) ? x : constants::WORLD_WIDTH - 1 - x;
                if (tileX < 0 || tileX >= constants::WORLD_WIDTH) continue;
                int sum = 0, count = 0;
                for (int dx = -1; dx <= 1; ++dx) {
                    int tx = tileX + dx;
                    if (tx >= 0 && tx < constants::WORLD_WIDTH) {
                        sum += m_surfaceHeight[tx];
                        ++count;
                    }
                }
                if (count > 0) smoothed[tileX] = sum / count;
            }
            for (int x = 0; x < oceanWidth; ++x) {
                int tileX = (side == 0) ? x : constants::WORLD_WIDTH - 1 - x;
                if (tileX >= 0 && tileX < constants::WORLD_WIDTH)
                    m_surfaceHeight[tileX] = smoothed[tileX];
            }
        }
    }
}

void WorldGenerator::repairOceanSurface() {
    int oceanMargin = 62;

    for (int x = 0; x < constants::WORLD_WIDTH; ++x) {
        if (x >= oceanMargin && x < constants::WORLD_WIDTH - oceanMargin) continue;

        int surfaceY = m_surfaceHeight[x];
        if (surfaceY < 5 || surfaceY >= constants::WORLD_HEIGHT - 10) continue;

        Biome biome = m_biomeMap[x];
        if (biome != Biome::Beach) continue;

        for (int y = 0; y < surfaceY; ++y) {
            m_world.setTile(x, y, TileId::Air);
            m_world.setWall(x, y, TileId::Air);
            m_world.setWater(x, y, 0);
            m_world.setLava(x, y, 0);
        }

        m_world.setTile(x, surfaceY, TileId::Sand);

        for (int y = surfaceY + 1; y <= surfaceY + 8 && y < constants::WORLD_HEIGHT; ++y) {
            m_world.setTile(x, y, TileId::Sand);
        }
    }
}

void WorldGenerator::generateWaterPools() {
    int spawnX = constants::WORLD_WIDTH / 2;
    int poolCount = constants::WORLD_WIDTH / 15;

    for (int i = 0; i < poolCount; ++i) {
        int cx = m_rng.range(20, constants::WORLD_WIDTH - 20);
        int surfaceY = m_surfaceHeight[cx];
        int depth = 18 + m_rng.range(0, constants::WORLD_HEIGHT - surfaceY - 45);
        int cy = surfaceY + depth;
        if (cy >= constants::WORLD_HEIGHT - 30) continue;

        if (std::abs(cx - spawnX) < SPAWN_SAFE_RADIUS) continue;

        TileId center = m_world.getTile(cx, cy);
        if (center != TileId::Stone && center != TileId::Dirt) continue;

        int poolW = 3 + m_rng.range(0, 5);
        int poolH = 1 + m_rng.range(0, 3);
        bool valid = true;

        for (int dy = -poolH - 1; dy <= 1 && valid; ++dy) {
            for (int dx = -poolW - 1; dx <= poolW + 1 && valid; ++dx) {
                int tx = cx + dx;
                int ty = cy + dy;
                if (!m_world.isInBounds(tx, ty)) { valid = false; break; }
                TileId t = m_world.getTile(tx, ty);
                if (t == TileId::Air || t == TileId::Wood || t == TileId::Planks) {
                    valid = false;
                }
            }
        }
        if (!valid) continue;

        for (int dy = -poolH - 1; dy <= 1; ++dy) {
            for (int dx = -poolW - 1; dx <= poolW + 1; ++dx) {
                float nx = static_cast<float>(dx) / static_cast<float>(poolW + 1);
                float ny = static_cast<float>(dy) / static_cast<float>(poolH + 1);
                float dist = nx * nx + ny * ny;
                if (dist > 1.0f) continue;
                if (dist > 0.7f && m_rng.range(0, 2) == 0) continue;

                int tx = cx + dx;
                int ty = cy + dy;
                if (!m_world.isInBounds(tx, ty)) continue;

                m_world.setTile(tx, ty, TileId::Air);
                if (dy >= -poolH / 2) {
                    m_world.setWater(tx, ty, MAX_LIQUID_LEVEL);
                }
            }
        }
    }
}

void WorldGenerator::generateLavaPools() {
    int underworldStart = constants::WORLD_HEIGHT - 50;
    int poolCount = constants::WORLD_WIDTH / 20;

    for (int i = 0; i < poolCount; ++i) {
        int cx = m_rng.range(10, constants::WORLD_WIDTH - 10);
        int cy = underworldStart + m_rng.range(0, 40);
        if (cy >= constants::WORLD_HEIGHT - 5) continue;

        TileId center = m_world.getTile(cx, cy);
        if (center == TileId::Air || center == TileId::Hellstone) continue;

        int poolW = 2 + m_rng.range(0, 4);
        int poolH = 1 + m_rng.range(0, 2);
        int lavaLevel = MAX_LIQUID_LEVEL - m_rng.range(0, 60);

        for (int dy = -poolH; dy <= 0; ++dy) {
            for (int dx = -poolW; dx <= poolW; ++dx) {
                int tx = cx + dx;
                int ty = cy + dy;
                if (!m_world.isInBounds(tx, ty)) continue;
                float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                if (dist > poolW + 0.5f) continue;
                if (dist > poolW - 1.0f && m_rng.range(0, 2) == 0) continue;
                m_world.setTile(tx, ty, TileId::Air);
                if (dy >= -1) {
                    m_world.setLava(tx, ty, lavaLevel);
                }
            }
        }
    }
}



