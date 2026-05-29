#include "Chunk.hpp"

Chunk::Chunk(int chunkX, int chunkY)
    : m_chunkX(chunkX), m_chunkY(chunkY) {
    m_tiles.fill(TileId::Air);
    m_walls.fill(TileId::Air);
    m_water.fill(0);
    m_lava.fill(0);
}

TileId Chunk::getTile(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return TileId::Air;
    return m_tiles[localY * constants::CHUNK_SIZE + localX];
}

void Chunk::setTile(int localX, int localY, TileId id) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    m_tiles[localY * constants::CHUNK_SIZE + localX] = id;
    m_dirty = true;
}

TileId Chunk::getWall(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return TileId::Air;
    return m_walls[localY * constants::CHUNK_SIZE + localX];
}

void Chunk::setWall(int localX, int localY, TileId id) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    m_walls[localY * constants::CHUNK_SIZE + localX] = id;
    m_dirty = true;
}

uint8_t Chunk::getWater(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return 0;
    return m_water[localY * constants::CHUNK_SIZE + localX];
}

void Chunk::setWater(int localX, int localY, uint8_t amount) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    m_water[localY * constants::CHUNK_SIZE + localX] = amount;
    m_dirty = true;
    if (amount > 0) m_hasLiquid = true;
}

uint8_t Chunk::getLava(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return 0;
    return m_lava[localY * constants::CHUNK_SIZE + localX];
}

void Chunk::setLava(int localX, int localY, uint8_t amount) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    m_lava[localY * constants::CHUNK_SIZE + localX] = amount;
    m_dirty = true;
    if (amount > 0) m_hasLiquid = true;
}
