#include "World.hpp"
#include "WorldGenerator.hpp"
#include "TileRegistry.hpp"
#include "core/Math.hpp"
#include <cassert>

bool World::isInBounds(int tileX, int tileY) const {
    return tileX >= 0 && tileX < constants::WORLD_WIDTH &&
           tileY >= 0 && tileY < constants::WORLD_HEIGHT;
}

TileId World::getTile(int tileX, int tileY) const {
    if (!isInBounds(tileX, tileY))
        return TileId::Air;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    auto it = m_chunks.find({cx, cy});
    if (it == m_chunks.end())
        return TileId::Air;
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    return it->second->getTile(lx, ly);
}

void World::setTile(int tileX, int tileY, TileId id) {
    if (!isInBounds(tileX, tileY))
        return;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    Chunk* chunk = getOrCreateChunk(cx, cy);
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    chunk->setTile(lx, ly, id);
}

TileId World::getWall(int tileX, int tileY) const {
    if (!isInBounds(tileX, tileY))
        return TileId::Air;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    auto it = m_chunks.find({cx, cy});
    if (it == m_chunks.end())
        return TileId::Air;
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    return it->second->getWall(lx, ly);
}

void World::setWall(int tileX, int tileY, TileId id) {
    if (!isInBounds(tileX, tileY))
        return;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    Chunk* chunk = getOrCreateChunk(cx, cy);
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    chunk->setWall(lx, ly, id);
}

bool World::isSolid(int tileX, int tileY) const {
    TileId id = getTile(tileX, tileY);
    if (id == TileId::Air) return false;
    return TileRegistry::instance().get(id).solid;
}

Chunk* World::getChunk(int chunkX, int chunkY) {
    auto it = m_chunks.find({chunkX, chunkY});
    return it != m_chunks.end() ? it->second.get() : nullptr;
}

const Chunk* World::getChunk(int chunkX, int chunkY) const {
    auto it = m_chunks.find({chunkX, chunkY});
    return it != m_chunks.end() ? it->second.get() : nullptr;
}

void World::ensureChunkExists(int chunkX, int chunkY) {
    auto key = std::make_pair(chunkX, chunkY);
    if (m_chunks.find(key) == m_chunks.end()) {
        m_chunks[key] = std::make_unique<Chunk>(chunkX, chunkY);
    }
}

Chunk* World::getOrCreateChunk(int chunkX, int chunkY) {
    auto key = std::make_pair(chunkX, chunkY);
    auto it = m_chunks.find(key);
    if (it == m_chunks.end()) {
        auto ptr = std::make_unique<Chunk>(chunkX, chunkY);
        it = m_chunks.emplace(key, std::move(ptr)).first;
    }
    return it->second.get();
}

void World::generate(unsigned int seed, ProgressCallback progress) {
    clear();
    m_seed = seed;
    WorldGenerator gen(*this, seed);
    gen.generate(progress);
}

void World::clear() {
    m_chunks.clear();
}
