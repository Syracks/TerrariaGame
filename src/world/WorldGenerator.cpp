#include "WorldGenerator.hpp"
#include "World.hpp"
#include "Tile.hpp"
#include "core/Constants.hpp"
#include <cmath>
#include <algorithm>
#include <random>
#include <iostream>

namespace {
    constexpr int SURFACE_SAFE_DEPTH = 26;
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

WorldGenerator::Biome WorldGenerator::getBiome(int tileX) const {
    float n = m_noise.octaveNoise(static_cast<float>(tileX) * 0.005f, 0.0f, 4, 0.5f, 1.0f);
    n = n * 0.5f + 0.5f;
    if (n < 0.10f) return Biome::Snow;
    if (n < 0.22f) return Biome::Desert;
    if (n < 0.38f) return Biome::Plains;
    if (n < 0.55f) return Biome::Forest;
    if (n < 0.78f) return Biome::Jungle;
    return Biome::Mushroom;
}

static float baseHeightForBiome(int biome) {
    switch (biome) {
        case 0:  return 0.28f;
        case 1:  return 0.40f;
        case 2:  return 0.32f;
        case 3:  return 0.34f;
        case 4:  return 0.33f;
        case 5:  return 0.36f;
        default: return 0.34f;
    }
}

int WorldGenerator::getSurfaceHeight(int tileX) const {
    float nBig   = m_noise.octaveNoise(tileX * 0.003f, 0.0f,    4, 0.5f, 1.0f);
    float nMed   = m_noise.octaveNoise(tileX * 0.008f, 100.0f,  3, 0.5f, 1.0f);
    float nSmall = m_noise.octaveNoise(tileX * 0.020f, 200.0f,  2, 0.5f, 1.0f);

    Biome biome = getBiome(tileX);
    float baseFrac = baseHeightForBiome(static_cast<int>(biome));
    const float blendRadius = 20.0f;
    float blendedFrac = 0.0f;
    float totalWeight = 0.0f;
    for (int dx = -static_cast<int>(blendRadius); dx <= static_cast<int>(blendRadius); ++dx) {
        int tx = tileX + dx;
        if (tx < 0 || tx >= constants::WORLD_WIDTH) continue;
        float w = 1.0f - std::abs(static_cast<float>(dx)) / blendRadius;
        blendedFrac += baseHeightForBiome(static_cast<int>(getBiome(tx))) * w;
        totalWeight += w;
    }
    if (totalWeight > 0.0f) baseFrac = blendedFrac / totalWeight;

    float heightFrac = baseFrac + nBig * 0.035f + nMed * 0.025f + nSmall * 0.010f;
    if (heightFrac < 0.20f) heightFrac = 0.20f;
    if (heightFrac > 0.55f) heightFrac = 0.55f;

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

    for (int pass = 0; pass < 3; ++pass) {
        std::vector<int> smoothed(constants::WORLD_WIDTH);
        for (int x = 0; x < constants::WORLD_WIDTH; ++x) {
            int sum = 0;
            int count = 0;
            for (int dx = -1; dx <= 1; ++dx) {
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

    for (int x = 1; x < constants::WORLD_WIDTH; ++x) {
        int diff = m_surfaceHeight[x] - m_surfaceHeight[x - 1];
        if (diff > 3) m_surfaceHeight[x] = m_surfaceHeight[x - 1] + 3;
        else if (diff < -3) m_surfaceHeight[x] = m_surfaceHeight[x - 1] - 3;
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

        int startY = surfaceY + SURFACE_SAFE_DEPTH + 20 + m_rng.range(0, 80);

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

            case Biome::Mushroom:
                surfaceTile = TileId::MushroomGrass;
                upperFill = TileId::Mud;
                lowerFill = TileId::Stone;
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
    if (progress) progress(0.10f);

    generateTerrain();
    if (progress) progress(0.25f);

    generateCaves();
    if (progress) progress(0.40f);

    generateWormCaves();
    if (progress) progress(0.50f);

    repairSurfaceLayer();
    if (progress) progress(0.58f);

    generateUndergroundCabins();
    if (progress) progress(0.68f);

    generateOreVeins();
    if (progress) progress(0.78f);

    generateUndergroundPockets();
    if (progress) progress(0.86f);

    repairSurfaceLayer();
    if (progress) progress(0.90f);

    generateTrees();
    if (progress) progress(1.0f);

    
    
}

void WorldGenerator::generateTerrain() {
    const int underworldTop = constants::WORLD_HEIGHT - 25;

    auto applyTransitionMix = [&](Biome biome, int tileX, TileId& fill) {
        if (!isNearBiomeEdge(tileX, 6) || m_rng.range(0, 4) != 0) return;
        for (int dx = -6; dx <= 6; ++dx) {
            int tx = tileX + dx;
            if (tx >= 0 && tx < constants::WORLD_WIDTH && m_biomeMap[tx] != biome) {
                switch (m_biomeMap[tx]) {
                    case Biome::Desert:   fill = TileId::Sand; break;
                    case Biome::Snow:     fill = TileId::SnowBlock; break;
                    case Biome::Jungle:   fill = TileId::Mud; break;
                    case Biome::Mushroom: fill = TileId::Mud; break;
                    default:              fill = TileId::Dirt; break;
                }
                break;
            }
        }
    };

    for (int tileX = 0; tileX < constants::WORLD_WIDTH; ++tileX) {
        Biome biome = m_biomeMap[tileX];
        int surfaceY = m_surfaceHeight[tileX];

        int dirtEnd = surfaceY + 25 + m_rng.range(0, 15);
        int stoneEnd = dirtEnd + 50 + m_rng.range(0, 20);

        for (int tileY = 0; tileY < constants::WORLD_HEIGHT; ++tileY) {
            if (tileY < surfaceY) {
                m_world.setTile(tileX, tileY, TileId::Air);
                continue;
            }
            if (tileY >= underworldTop) {
                m_world.setTile(tileX, tileY, TileId::Hellstone);
                continue;
            }
            if (tileY == surfaceY) {
                TileId fill;
                switch (biome) {
                    case Biome::Snow:     fill = TileId::SnowBlock; break;
                    case Biome::Desert:   fill = TileId::Sand; break;
                    case Biome::Mushroom: fill = TileId::MushroomGrass; break;
                    case Biome::Jungle:   fill = TileId::JungleGrass; break;
                    default:              fill = TileId::Grass; break;
                }
                applyTransitionMix(biome, tileX, fill);
                m_world.setTile(tileX, tileY, fill);
                continue;
            }

            TileId fill;
            if (tileY < surfaceY + 3) {
                if (biome == Biome::Desert)       fill = TileId::Sand;
                else if (biome == Biome::Snow)    fill = TileId::SnowBlock;
                else if (biome == Biome::Jungle)  fill = TileId::Mud;
                else if (biome == Biome::Mushroom) fill = TileId::Mud;
                else                              fill = TileId::Dirt;
                applyTransitionMix(biome, tileX, fill);
            } else if (tileY < dirtEnd) {
                switch (biome) {
                    case Biome::Desert:   fill = TileId::Sand; break;
                    case Biome::Jungle:   fill = TileId::Mud; break;
                    case Biome::Mushroom: fill = TileId::Mud; break;
                    default:              fill = TileId::Dirt; break;
                }
                if (biome == Biome::Snow && tileY < surfaceY + 6)
                    fill = TileId::SnowBlock;
            } else if (tileY < stoneEnd) {
                if (biome == Biome::Desert)       fill = TileId::Sand;
                else if (biome == Biome::Jungle)  fill = TileId::Mud;
                else if (biome == Biome::Mushroom) fill = (tileY < dirtEnd + 15) ? TileId::Mud : TileId::Stone;
                else                              fill = TileId::Stone;
            } else {
                fill = TileId::Stone;
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
        if (std::abs(tileX - spawnX) < SPAWN_SAFE_RADIUS) {
            continue;
        }

        Biome biome = m_biomeMap[tileX];
        int surfaceY = m_surfaceHeight[tileX];
        int caveStartY = surfaceY + SURFACE_SAFE_DEPTH;
        if (caveStartY < 6) caveStartY = 6;

        for (int tileY = caveStartY; tileY < constants::WORLD_HEIGHT - 5; ++tileY) {
            TileId tile = m_world.getTile(tileX, tileY);
            if (!isReplaceableForCave(tile)) continue;

            float caveNoise = m_noise.octaveNoise(
                tileX * 0.030f, tileY * 0.030f, 3, 0.5f, 1.0f);

            float tunnelNoise = m_noise.octaveNoise(
                tileX * 0.040f, tileY * 0.020f, 2, 0.5f, 1.0f);

            float cavernNoise = m_noise.octaveNoise(
                tileX * 0.012f, tileY * 0.012f, 2, 0.5f, 1.0f);

            float depthFrac = static_cast<float>(tileY - caveStartY) /
                              static_cast<float>(constants::WORLD_HEIGHT - caveStartY - 5);
            float threshold = 0.42f - depthFrac * 0.18f;

            float tunnelContrib = std::max(0.0f, tunnelNoise - 0.25f) * 0.12f;
            float cavernContrib = std::max(0.0f, cavernNoise - 0.55f) * 0.22f;

            float biomeBonus = 0.0f;
            if (biome == Biome::Mushroom) biomeBonus = 0.12f;
            if (biome == Biome::Jungle) biomeBonus = 0.05f;

            float combined = caveNoise + tunnelContrib + cavernContrib + biomeBonus;

            if (combined > threshold) {
                m_world.setTile(tileX, tileY, TileId::Air);
            }
        }
    }
}

void WorldGenerator::generateOreVeins() {
    int veinCount = constants::WORLD_WIDTH * constants::WORLD_HEIGHT / 700;

    for (int i = 0; i < veinCount; ++i) {
        int cx = m_rng.range(0, constants::WORLD_WIDTH);
        int cy = 50 + m_rng.range(0, constants::WORLD_HEIGHT - 100);
        int surfaceY = m_surfaceHeight[cx];
        int depth = cy - surfaceY;

        TileId oreType;
        int minDepth, maxSize;
        int r = m_rng.range(0, 100);
        if (r < 40)      { oreType = TileId::CopperOre; minDepth = 8;  maxSize = 8; }
        else if (r < 70) { oreType = TileId::IronOre;   minDepth = 25; maxSize = 6; }
        else if (r < 85) { oreType = TileId::GoldOre;   minDepth = 45; maxSize = 5; }
        else             { oreType = TileId::CopperOre; minDepth = 8;  maxSize = 4; }

        if (depth < minDepth) continue;

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
    return surface == TileId::Grass || surface == TileId::MushroomGrass ||
           surface == TileId::JungleGrass || surface == TileId::Sand;
}

void WorldGenerator::generateTrees() {
    int spawnX = constants::WORLD_WIDTH / 2;
    int lastTreeX = -9999;

    for (int tileX = 3; tileX < constants::WORLD_WIDTH - 3; ++tileX) {
        Biome biome = m_biomeMap[tileX];
        int surfaceY = m_surfaceHeight[tileX];

        if (std::abs(tileX - spawnX) < 12) continue;

        if (tileX - lastTreeX < 10) continue;

        TileId surfaceTile = m_world.getTile(tileX, surfaceY);
        if (!canGrowTreeOn(surfaceTile)) continue;
        if (m_world.getTile(tileX, surfaceY - 1) != TileId::Air) continue;
        if (biome == Biome::Forest) {
            if (m_rng.range(0, 7) != 0) continue;
        } else if (biome == Biome::Plains) {
            if (m_rng.range(0, 18) != 0) continue;
        } else if (biome == Biome::Mushroom) {
            if (m_rng.range(0, 6) != 0) continue;
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
            int trunkHeight = 6 + m_rng.range(0, 4);
            int canopyRadius = 2 + m_rng.range(0, 2);
            bool blocked = false;
            for (int dy = 1; dy <= trunkHeight + 3; ++dy) {
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
                        if (m_world.getTile(lx, ly) == TileId::Leaf) {
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
        if (biome == Biome::Mushroom) {
            int stemHeight = 3 + m_rng.range(0, 3);
            int capRadius = 2 + m_rng.range(0, 2);
            bool blocked = false;
            for (int dy = 1; dy <= stemHeight + capRadius + 1; ++dy) {
                int y = surfaceY - dy;
                if (y < 0 || m_world.getTile(tileX, y) != TileId::Air) {
                    blocked = true;
                    break;
                }
            }
            if (blocked) continue;
            for (int dx = -capRadius - 1; dx <= capRadius + 1; ++dx) {
                for (int dy = -capRadius - 1; dy <= 0; ++dy) {
                    int lx = tileX + dx;
                    int ly = surfaceY - stemHeight + dy;
                    if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                        if (m_world.getTile(lx, ly) == TileId::Leaf) {
                            blocked = true;
                            break;
                        }
                    }
                }
                if (blocked) break;
            }
            if (blocked) continue;
            for (int dy = 1; dy <= stemHeight; ++dy) {
                m_world.setTile(tileX, surfaceY - dy, TileId::Wood);
            }
            for (int dy = -capRadius; dy <= 0; ++dy) {
                int r = capRadius + dy;
                for (int dx = -r; dx <= r; ++dx) {
                    if (dx == 0 && dy >= 0) continue;
                    int lx = tileX + dx;
                    int ly = surfaceY - stemHeight + dy;
                    if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                        if (m_world.getTile(lx, ly) == TileId::Air) {
                            if (std::abs(dx) == r && m_rng.range(0, 2) == 0) continue;
                            m_world.setTile(lx, ly, TileId::Leaf);
                        }
                    }
                }
            }
            lastTreeX = tileX;
            continue;
        }
        
        int trunkHeight = 4 + m_rng.range(0, 3);
        bool blocked = false;
        for (int dy = 1; dy <= trunkHeight + 2; ++dy) {
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
                    if (m_world.getTile(lx, ly) == TileId::Leaf) {
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
            lastTreeX = tileX;
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

        if (biome == Biome::Desert || biome == Biome::Snow || biome == Biome::Mushroom)
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

void WorldGenerator::generateFloatingIslands() {
    int spawnX = constants::WORLD_WIDTH / 2;

    int targetCount = std::max(1, constants::WORLD_WIDTH / 600);
    int placed = 0;

    for (int attempt = 0; attempt < targetCount * 40 && placed < targetCount; ++attempt) {
        int margin = 120;

        if (constants::WORLD_WIDTH <= margin * 2)
            break;

        int cx = margin + m_rng.range(0, constants::WORLD_WIDTH - margin * 2);

        if (std::abs(cx - spawnX) < 160)
            continue;

        Biome biome = m_biomeMap[cx];
        Biome islandBiome = (biome == Biome::Desert) ? Biome::Desert : Biome::Forest;
        bool isDesert = (islandBiome == Biome::Desert);

        int surfaceY = m_surfaceHeight[cx];

        int maxIslandY = std::max(8, surfaceY - 90);
        if (maxIslandY <= 8)
            continue;

        int islandY = m_rng.range(6, maxIslandY);

        int islandWidth = 6 + m_rng.range(0, 8);
        int islandHeight = 3 + m_rng.range(0, 3);

        bool canPlace = true;
        for (int dx = -islandWidth - 1; dx <= islandWidth + 1; ++dx) {
            for (int dy = -islandHeight - 1; dy <= 1; ++dy) {
                int tx = cx + dx;
                int ty = islandY + dy;
                if (tx >= 0 && tx < constants::WORLD_WIDTH &&
                    ty >= 0 && ty < constants::WORLD_HEIGHT) {
                    if (m_world.getTile(tx, ty) != TileId::Air) {
                        canPlace = false;
                        break;
                    }
                }
            }
            if (!canPlace) break;
        }
        if (!canPlace) continue;

        for (int dx = -islandWidth; dx <= islandWidth; ++dx) {
            int tx = cx + dx;
            if (tx < 0 || tx >= constants::WORLD_WIDTH) continue;
            float edgeFactor = 1.0f - static_cast<float>(std::abs(dx)) /
                                        static_cast<float>(islandWidth);

            for (int dy = -islandHeight; dy <= 0; ++dy) {
                int ty = islandY + dy;
                if (ty < 0 || ty >= constants::WORLD_HEIGHT) continue;

                float depthFactor = 1.0f - static_cast<float>(-dy) /
                                            static_cast<float>(islandHeight);
                float placeChance = edgeFactor * (1.0f - depthFactor * 0.3f);
                if (edgeFactor < 0.3f) placeChance *= 0.5f;
                if (std::abs(dx) > islandWidth - 1 || dy == -islandHeight) {
                    if (m_rng.rangeF(0.0f, 1.0f) > placeChance) continue;
                }

                if (dy == 0) {
                    m_world.setTile(tx, ty, isDesert ? TileId::Sand : TileId::Grass);
                } else if (dy > -3) {
                    m_world.setTile(tx, ty, isDesert ? TileId::Sand : TileId::Dirt);
                } else {
                    m_world.setTile(tx, ty, TileId::Stone);
                }
            }
        }

        if (m_rng.range(0, 2) == 0 && islandHeight > 4 && islandWidth > 10) {
            int pocketCX = cx + m_rng.range(-2, 2);
            int pocketCY = islandY - islandHeight / 2;
            int pocketR = 1 + m_rng.range(0, 2);
            for (int dy = -pocketR; dy <= pocketR; ++dy) {
                for (int dx = -pocketR; dx <= pocketR; ++dx) {
                    float dist = std::sqrt(static_cast<float>(dx * dx + dy * dy));
                    if (dist <= static_cast<float>(pocketR) + 0.5f) {
                        int tx = pocketCX + dx;
                        int ty = pocketCY + dy;
                        if (m_world.isInBounds(tx, ty) && m_world.getTile(tx, ty) != TileId::Air) {
                            m_world.setTile(tx, ty, TileId::Air);
                        }
                    }
                }
            }
        }

        if (!isDesert && m_rng.range(0, 2) == 0) {
            int treeX = cx + m_rng.range(-1, 2);
            if (m_world.getTile(treeX, islandY) == TileId::Grass &&
                m_world.getTile(treeX, islandY - 1) == TileId::Air) {
                int trunkHeight = 2 + m_rng.range(0, 2);
                for (int dy = 1; dy <= trunkHeight; ++dy) {
                    m_world.setTile(treeX, islandY - dy, TileId::Wood);
                }
                for (int dy = -1; dy <= 0; ++dy) {
                    for (int dx = -1; dx <= 1; ++dx) {
                        int lx = treeX + dx;
                        int ly = islandY - trunkHeight + dy;
                        if (lx >= 0 && lx < constants::WORLD_WIDTH && ly >= 0) {
                            if (dx == 0 && dy >= 0) continue;
                            if (m_world.getTile(lx, ly) == TileId::Air) {
                                if (std::abs(dx) == 1 && dy == -1 && m_rng.range(0, 2) == 0)
                                    continue;
                                m_world.setTile(lx, ly, TileId::Leaf);
                            }
                        }
                    }
                }
            }
        }

        placed++;
    }
}
