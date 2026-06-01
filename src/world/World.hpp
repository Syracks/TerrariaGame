#pragma once

#include "Chunk.hpp"
#include "Tile.hpp"
#include "items/ItemStack.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>
#include <functional>
#include <array>

constexpr int CHEST_SLOTS = 25;

class World {
public:
    World() = default;
    ~World() = default;

    TileId getTile(int tileX, int tileY) const;
    void setTile(int tileX, int tileY, TileId id);
    TileId getWall(int tileX, int tileY) const;
    void setWall(int tileX, int tileY, TileId id);
    uint8_t getWater(int tileX, int tileY) const;
    void setWater(int tileX, int tileY, uint8_t amount);
    uint8_t getLava(int tileX, int tileY) const;
    void setLava(int tileX, int tileY, uint8_t amount);
    bool isSolid(int tileX, int tileY) const;
    bool isInBounds(int tileX, int tileY) const;
    bool isDoorOpen(int tileX, int tileY) const;
    void setDoorOpen(int tileX, int tileY, bool open);

    struct pair_hash {
        std::size_t operator()(const std::pair<int,int>& p) const {
            std::size_t h1 = std::hash<int>()(p.first);
            std::size_t h2 = std::hash<int>()(p.second);
            return h1 ^ (h2 + 0x9e3779b9 + (h1 << 6) + (h1 >> 2));
        }
    };

    std::array<ItemStack, CHEST_SLOTS>& getChest(int tileX, int tileY);
    const std::array<ItemStack, CHEST_SLOTS>& getChestConst(int tileX, int tileY) const;
    void removeChest(int tileX, int tileY);
    const std::unordered_map<std::pair<int,int>, std::array<ItemStack, CHEST_SLOTS>, pair_hash>& getChests() const { return m_chests; }

    using ChunkMap = std::unordered_map<std::pair<int,int>, std::unique_ptr<Chunk>, pair_hash>;
    const ChunkMap& getChunks() const { return m_chunks; }

    using ProgressCallback = std::function<void(float)>;
    void generate(unsigned int seed, ProgressCallback progress = nullptr);
    void clear();

    Chunk* getChunk(int chunkX, int chunkY);
    const Chunk* getChunk(int chunkX, int chunkY) const;

    void getChunksWithLiquid(std::vector<Chunk*>& outChunks);

    int getWorldWidth() const { return constants::WORLD_WIDTH; }
    int getWorldHeight() const { return constants::WORLD_HEIGHT; }

    Biome getBiome(int tileX) const;
    int getSurfaceHeight(int tileX) const;
    void setBiomeData(const std::vector<Biome>& biomes, const std::vector<int>& heights);

private:
    ChunkMap m_chunks;
    unsigned int m_seed = 0;
    std::vector<Biome> m_biomeMap;
    std::vector<int> m_surfaceHeight;
    std::unordered_map<std::pair<int,int>, std::array<ItemStack, CHEST_SLOTS>, pair_hash> m_chests;

    void ensureChunkExists(int chunkX, int chunkY);
    Chunk* getOrCreateChunk(int chunkX, int chunkY);
};
