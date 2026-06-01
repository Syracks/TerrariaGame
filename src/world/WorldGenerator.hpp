#pragma once

#include "Tile.hpp"
#include <cstdint>
#include <vector>
#include <functional>

class World;

class WorldGenerator {
public:
    using ProgressCallback = std::function<void(float)>;

    WorldGenerator(World& world, unsigned int seed);
    void generate(ProgressCallback progress = nullptr);

private:
    struct PRNG {
        unsigned int state;
        explicit PRNG(unsigned int seed);
        int range(int min, int max);
        float rangeF(float min, float max);
    };

    struct PerlinNoise {
        std::vector<int> perm;
        PerlinNoise(unsigned int seed);
        float noise(float x, float y) const;
        float octaveNoise(float x, float y, int octaves, float persistence, float scale) const;
    private:
        float fade(float t) const;
        float lerp(float a, float b, float t) const;
        float grad(int hash, float x, float y) const;
    };

    World& m_world;
    unsigned int m_seed;
    PerlinNoise m_noise;
    PRNG m_rng;

    std::vector<int> m_surfaceHeight;
    std::vector<Biome> m_biomeMap;

    Biome getBiome(int tileX) const;
    int getSurfaceHeight(int tileX) const;

    void buildWorldMaps();
    void protectSpawnArea();
    void rebuildSpawnArea();
    void repairSurfaceLayer();
    void generateWormCaves();
    void carveCircle(int cx, int cy, int radius);
    bool hasNearbyAir(int cx, int cy, int radius) const;
    bool isNearBiomeEdge(int x, int radius) const;

    void generateTerrain();
    void generateCaves();
    void generateOreVeins();
    void generateTrees();
    void generateUndergroundPockets();
    void generateFloatingIslands();
    void generateUndergroundCabins();
    void generateOceans();
    void repairOceanTransitions();
    void generateWaterPools();
    void generateLavaPools();
    void postProcess();

    bool isSolidBelow(int tileX, int tileY) const;
    bool isReplaceableForCave(TileId id) const;
};
