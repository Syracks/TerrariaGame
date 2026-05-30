#include "Chunk.hpp"

Chunk::Chunk(int chunkX, int chunkY)
    : m_chunkX(chunkX), m_chunkY(chunkY) {
    m_tiles.fill(TileId::Air);
    m_walls.fill(TileId::Air);
    m_liquid.fill(0);
    m_doorOpen.fill(false);
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

namespace {
    uint8_t packLiquid(uint8_t amount, bool isLava) {
        return (amount / 2) | (isLava ? 0x80 : 0);
    }
    uint8_t unpackLevel(uint8_t packed) {
        return (packed & 0x7F) * 2;
    }
    bool unpackIsLava(uint8_t packed) {
        return (packed & 0x80) != 0;
    }
}

uint8_t Chunk::getWater(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return 0;
    uint8_t packed = m_liquid[localY * constants::CHUNK_SIZE + localX];
    return (packed != 0 && !unpackIsLava(packed)) ? unpackLevel(packed) : 0;
}

void Chunk::setWater(int localX, int localY, uint8_t amount) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    int idx = localY * constants::CHUNK_SIZE + localX;
    m_liquid[idx] = (amount > 0) ? packLiquid(amount, false) : 0;
    m_dirty = true;
    if (amount > 0) m_hasLiquid = true;
}

uint8_t Chunk::getLava(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return 0;
    uint8_t packed = m_liquid[localY * constants::CHUNK_SIZE + localX];
    return (packed != 0 && unpackIsLava(packed)) ? unpackLevel(packed) : 0;
}

void Chunk::setLava(int localX, int localY, uint8_t amount) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    int idx = localY * constants::CHUNK_SIZE + localX;
    m_liquid[idx] = (amount > 0) ? packLiquid(amount, true) : 0;
    m_dirty = true;
    if (amount > 0) m_hasLiquid = true;
}

bool Chunk::isDoorOpen(int localX, int localY) const {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return false;
    return m_doorOpen[localY * constants::CHUNK_SIZE + localX];
}

void Chunk::setDoorOpen(int localX, int localY, bool open) {
    if (localX < 0 || localX >= constants::CHUNK_SIZE ||
        localY < 0 || localY >= constants::CHUNK_SIZE)
        return;
    m_doorOpen[localY * constants::CHUNK_SIZE + localX] = open;
    m_dirty = true;
}
