#pragma once

#include "Tile.hpp"
#include "core/Constants.hpp"
#include <array>
#include <cstdint>
#include <vector>

class Chunk {
public:
    Chunk(int chunkX, int chunkY);

    TileId getTile(int localX, int localY) const;
    void setTile(int localX, int localY, TileId id);
    TileId getWall(int localX, int localY) const;
    void setWall(int localX, int localY, TileId id);

    uint8_t getWater(int localX, int localY) const;
    void setWater(int localX, int localY, uint8_t amount);
    uint8_t getLava(int localX, int localY) const;
    void setLava(int localX, int localY, uint8_t amount);

    bool isDirty() const { return m_dirty; }
    void markClean() { m_dirty = false; }
    bool hasLiquid() const { return m_hasLiquid; }
    void setHasLiquid(bool v) { m_hasLiquid = v; }
    int getChunkX() const { return m_chunkX; }
    int getChunkY() const { return m_chunkY; }

    void setRawTile(int index, TileId id) { m_tiles[index] = id; }
    void setRawWater(int index, uint8_t amount) { m_water[index] = amount; }
    void setRawLava(int index, uint8_t amount) { m_lava[index] = amount; }

private:
    int m_chunkX, m_chunkY;
    bool m_dirty = false;
    bool m_hasLiquid = false;
    std::array<TileId, constants::CHUNK_SIZE * constants::CHUNK_SIZE> m_tiles;
    std::array<TileId, constants::CHUNK_SIZE * constants::CHUNK_SIZE> m_walls;
    std::array<uint8_t, constants::CHUNK_SIZE * constants::CHUNK_SIZE> m_water;
    std::array<uint8_t, constants::CHUNK_SIZE * constants::CHUNK_SIZE> m_lava;
};
