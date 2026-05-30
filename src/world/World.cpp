#include "World.hpp"
#include "WorldGenerator.hpp"
#include "TileRegistry.hpp"
#include "core/Math.hpp"
#include <cassert>
#include <vector>

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

uint8_t World::getWater(int tileX, int tileY) const {
    if (!isInBounds(tileX, tileY))
        return 0;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    auto it = m_chunks.find({cx, cy});
    if (it == m_chunks.end())
        return 0;
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    return it->second->getWater(lx, ly);
}

void World::setWater(int tileX, int tileY, uint8_t amount) {
    if (!isInBounds(tileX, tileY))
        return;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    Chunk* chunk = getOrCreateChunk(cx, cy);
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    chunk->setWater(lx, ly, amount);
}

uint8_t World::getLava(int tileX, int tileY) const {
    if (!isInBounds(tileX, tileY))
        return 0;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    auto it = m_chunks.find({cx, cy});
    if (it == m_chunks.end())
        return 0;
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    return it->second->getLava(lx, ly);
}

void World::setLava(int tileX, int tileY, uint8_t amount) {
    if (!isInBounds(tileX, tileY))
        return;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    Chunk* chunk = getOrCreateChunk(cx, cy);
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    chunk->setLava(lx, ly, amount);
}

bool World::isSolid(int tileX, int tileY) const {
    TileId id = getTile(tileX, tileY);
    if (id == TileId::Air) return false;
    if (id == TileId::Door && isDoorOpen(tileX, tileY)) return false;
    return TileRegistry::instance().get(id).solid;
}

bool World::isDoorOpen(int tileX, int tileY) const {
    if (!isInBounds(tileX, tileY))
        return false;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    auto it = m_chunks.find({cx, cy});
    if (it == m_chunks.end())
        return false;
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    return it->second->isDoorOpen(lx, ly);
}

void World::setDoorOpen(int tileX, int tileY, bool open) {
    if (!isInBounds(tileX, tileY))
        return;
    int cx = math::chunkFromTile(tileX);
    int cy = math::chunkFromTile(tileY);
    Chunk* chunk = getOrCreateChunk(cx, cy);
    int lx = math::localTileInChunk(tileX);
    int ly = math::localTileInChunk(tileY);
    chunk->setDoorOpen(lx, ly, open);
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

void World::getChunksWithLiquid(std::vector<Chunk*>& outChunks) {
    for (auto& [key, chunk] : m_chunks) {
        if (chunk->hasLiquid()) {
            outChunks.push_back(chunk.get());
        }
    }
}

Biome World::getBiome(int tileX) const {
    if (m_biomeMap.empty()) return Biome::Forest;
    if (tileX < 0) return m_biomeMap[0];
    if (tileX >= static_cast<int>(m_biomeMap.size())) return m_biomeMap.back();
    return m_biomeMap[tileX];
}

int World::getSurfaceHeight(int tileX) const {
    if (m_surfaceHeight.empty()) return 0;
    if (tileX < 0) return m_surfaceHeight[0];
    if (tileX >= static_cast<int>(m_surfaceHeight.size())) return m_surfaceHeight.back();
    return m_surfaceHeight[tileX];
}

void World::setBiomeData(const std::vector<Biome>& biomes, const std::vector<int>& heights) {
    m_biomeMap = biomes;
    m_surfaceHeight = heights;
}

void World::clear() {
    m_chunks.clear();
    m_biomeMap.clear();
    m_surfaceHeight.clear();
    m_chests.clear();
}

std::array<ItemStack, CHEST_SLOTS>& World::getChest(int tileX, int tileY) {
    auto key = std::make_pair(tileX, tileY);
    auto it = m_chests.find(key);
    if (it == m_chests.end()) {
        std::array<ItemStack, CHEST_SLOTS> empty{};
        empty.fill({TileId::Air, 0});
        it = m_chests.emplace(key, empty).first;
    }
    return it->second;
}

void World::removeChest(int tileX, int tileY) {
    m_chests.erase(std::make_pair(tileX, tileY));
}

const std::array<ItemStack, CHEST_SLOTS>& World::getChestConst(int tileX, int tileY) const {
    static const std::array<ItemStack, CHEST_SLOTS> empty{};
    auto it = m_chests.find(std::make_pair(tileX, tileY));
    if (it == m_chests.end())
        return empty;
    return it->second;
}
