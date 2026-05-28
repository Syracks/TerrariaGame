#pragma once

#include "Chunk.hpp"
#include "Tile.hpp"
#include <unordered_map>
#include <memory>
#include <string>
#include <utility>
#include <functional>

class World {
public:
    World() = default;
    ~World() = default;

    TileId getTile(int tileX, int tileY) const;
    void setTile(int tileX, int tileY, TileId id);
    TileId getWall(int tileX, int tileY) const;
    void setWall(int tileX, int tileY, TileId id);
    bool isSolid(int tileX, int tileY) const;
    bool isInBounds(int tileX, int tileY) const;

    using ProgressCallback = std::function<void(float)>;
    void generate(unsigned int seed, ProgressCallback progress = nullptr);
    void clear();

    Chunk* getChunk(int chunkX, int chunkY);
    const Chunk* getChunk(int chunkX, int chunkY) const;

    int getWorldWidth() const { return constants::WORLD_WIDTH; }
    int getWorldHeight() const { return constants::WORLD_HEIGHT; }

    struct pair_hash {
        std::size_t operator()(const std::pair<int,int>& p) const {
            return std::hash<int>()(p.first) ^ (std::hash<int>()(p.second) << 1);
        }
    };

    using ChunkMap = std::unordered_map<std::pair<int,int>, std::unique_ptr<Chunk>, pair_hash>;
    const ChunkMap& getChunks() const { return m_chunks; }

private:
    ChunkMap m_chunks;
    unsigned int m_seed = 0;

    void ensureChunkExists(int chunkX, int chunkY);
    Chunk* getOrCreateChunk(int chunkX, int chunkY);
};
