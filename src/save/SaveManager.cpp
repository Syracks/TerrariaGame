#include "SaveManager.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/Tile.hpp"
#include "world/TileRegistry.hpp"
#include "entities/Player.hpp"
#include "items/Inventory.hpp"
#include "core/Math.hpp"
#include "core/Constants.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <sys/stat.h>
#include <cerrno>
#include <cstdlib>

static bool ensureDir(const std::string& path) {
    if (mkdir(path.c_str(), 0755) != 0 && errno != EEXIST) {
        std::cerr << "Failed to create directory: " << path << " (errno=" << errno << ")" << std::endl;
        return false;
    }
    return true;
}

std::string SaveManager::getSlotPath(int slot) {
    return "saves/slot" + std::to_string(slot);
}

std::string SaveManager::getSlotMetaPath(int slot) {
    return getSlotPath(slot) + "/meta.txt";
}

std::string SaveManager::getSlotDataPath(int slot) {
    return getSlotPath(slot) + "/world.dat";
}

SlotInfo SaveManager::getSlotInfo(int slot) {
    SlotInfo info;
    std::string path = getSlotMetaPath(slot);
    std::ifstream file(path);
    if (!file.is_open()) {
        info.occupied = false;
        return info;
    }
    info.occupied = true;
    std::getline(file, info.name);
    std::string sizeStr;
    std::getline(file, sizeStr);
    if (sizeStr == "Small") info.size = WorldSize::Small;
    else if (sizeStr == "Large") info.size = WorldSize::Large;
    else info.size = WorldSize::Medium;
    std::string seedStr;
    std::getline(file, seedStr);
    try {
        info.seed = static_cast<unsigned int>(std::stoul(seedStr));
    } catch (...) {
        info.seed = 0;
    }
    std::string diffStr;
    std::getline(file, diffStr);
    if (diffStr == "Hardcore") info.difficulty = Difficulty::Hardcore;
    else info.difficulty = Difficulty::Normal;
    return info;
}

bool SaveManager::saveSlotMeta(int slot, const SlotInfo& info) {
    ensureDir("saves");
    ensureDir(getSlotPath(slot));
    std::ofstream file(getSlotMetaPath(slot));
    if (!file.is_open()) return false;
    file << info.name << "\n";
    file << worldSizeName(info.size) << "\n";
    file << info.seed << "\n";
    file << difficultyName(info.difficulty) << "\n";
    return true;
}

bool SaveManager::saveSlot(int slot, const World& world, const Player& player,
                           const std::string& name, WorldSize size, unsigned int seed,
                           Difficulty difficulty, float dayTime) {
    ensureDir("saves");
    ensureDir(getSlotPath(slot));
    SlotInfo info;
    info.name = name;
    info.size = size;
    info.difficulty = difficulty;
    info.seed = seed;
    info.occupied = true;
    if (!saveSlotMeta(slot, info))
        return false;
    return save(world, player, getSlotDataPath(slot), dayTime);
}

bool SaveManager::loadSlot(int slot, World& world, Player& player, float& dayTime) {
    return load(world, player, getSlotDataPath(slot), dayTime);
}

void SaveManager::deleteSlot(int slot) {
    std::remove(getSlotMetaPath(slot).c_str());
    std::remove(getSlotDataPath(slot).c_str());
}

std::array<SlotInfo, SaveManager::SLOT_COUNT> SaveManager::listSlots() {
    std::array<SlotInfo, SLOT_COUNT> slots;
    for (int i = 0; i < SLOT_COUNT; ++i) {
        slots[i] = getSlotInfo(i);
    }
    return slots;
}

bool SaveManager::save(const World& world, const Player& player, const std::string& filepath, float dayTime) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open save file for writing: " << filepath << std::endl;
        return false;
    }

    file << "TERRARIA_SAVE_V1\n";
    file << "WORLD " << world.getWorldWidth() << " " << world.getWorldHeight() << "\n";
    file << "PLAYER " << player.getPosition().x << " " << player.getPosition().y << "\n";
    file << "DAY_TIME " << dayTime << "\n";

    for (const auto& [key, chunk] : world.getChunks()) {
        if (!chunk->isDirty())
            continue;

        int baseTileX = chunk->getChunkX() * constants::CHUNK_SIZE;
        int baseTileY = chunk->getChunkY() * constants::CHUNK_SIZE;

        for (int ly = 0; ly < constants::CHUNK_SIZE; ++ly) {
            for (int lx = 0; lx < constants::CHUNK_SIZE; ++lx) {
                TileId id = chunk->getTile(lx, ly);
                if (id != TileId::Air) {
                    int worldX = baseTileX + lx;
                    int worldY = baseTileY + ly;
                    file << "TILE " << worldX << " " << worldY << " " << static_cast<int>(id) << "\n";
                }
                uint8_t water = chunk->getWater(lx, ly);
                if (water > 0) {
                    int worldX = baseTileX + lx;
                    int worldY = baseTileY + ly;
                    file << "WATER " << worldX << " " << worldY << " " << static_cast<int>(water) << "\n";
                }
                uint8_t lava = chunk->getLava(lx, ly);
                if (lava > 0) {
                    int worldX = baseTileX + lx;
                    int worldY = baseTileY + ly;
                    file << "LAVA " << worldX << " " << worldY << " " << static_cast<int>(lava) << "\n";
                }
                TileId wall = chunk->getWall(lx, ly);
                if (wall != TileId::Air) {
                    int worldX = baseTileX + lx;
                    int worldY = baseTileY + ly;
                    file << "WALL " << worldX << " " << worldY << " " << static_cast<int>(wall) << "\n";
                }
                if (chunk->isDoorOpen(lx, ly)) {
                    int worldX = baseTileX + lx;
                    int worldY = baseTileY + ly;
                    file << "DOOR_OPEN " << worldX << " " << worldY << "\n";
                }
            }
        }
    }

    const auto& inventory = player.getInventory();
    const auto& slots = inventory.getSlots();
    for (int i = 0; i < constants::INVENTORY_SLOTS; ++i) {
        if (slots[i].tileId != TileId::Air && slots[i].count > 0) {
            file << "INVENTORY " << i << " " << static_cast<int>(slots[i].tileId) << " " << slots[i].count << "\n";
        }
    }

    file << "SELECTED " << inventory.getSelectedIndex() << "\n";

    for (const auto& [key, chest] : world.getChests()) {
        int tx = key.first;
        int ty = key.second;
        for (int i = 0; i < CHEST_SLOTS; ++i) {
            if (chest[i].tileId != TileId::Air && chest[i].count > 0) {
                file << "CHEST " << tx << " " << ty << " " << i << " "
                     << static_cast<int>(chest[i].tileId) << " " << chest[i].count << "\n";
            }
        }
    }

    file << "END\n";
    file.flush();
    if (!file.good()) {
        std::cerr << "Failed to write save file (disk full?): " << filepath << std::endl;
        return false;
    }
    return true;
}

bool SaveManager::load(World& world, Player& player, const std::string& filepath, float& dayTime) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "No save file found: " << filepath << std::endl;
        return false;
    }

    world.clear();
    std::string line;

    std::getline(file, line);
    if (line == "TERRARIA_SAVE_V1") {
    } else {
        file.clear();
        file.seekg(0);
    }

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        if (!(iss >> keyword)) continue;

        if (keyword == "WORLD") {
            int w, h;
            if (iss >> w >> h && w > 0 && h > 0 && w <= 8192 && h <= 4096) {
                constants::WORLD_WIDTH = w;
                constants::WORLD_HEIGHT = h;
            }
        } else if (keyword == "DAY_TIME") {
            float dt;
            if (iss >> dt && dt >= 0.0f) dayTime = dt;
        } else if (keyword == "PLAYER") {
            float x, y;
            if (iss >> x >> y) {
                player.setPosition({x, y});
                player.setVelocity({0, 0});
            }
        } else if (keyword == "TILE") {
            int tx, ty, tileId;
            if (iss >> tx >> ty >> tileId && tileId >= 0 && tileId <= 55)
                world.setTile(tx, ty, static_cast<TileId>(tileId));
        } else if (keyword == "WATER") {
            int tx, ty, amount;
            if (iss >> tx >> ty >> amount)
                world.setWater(tx, ty, static_cast<uint8_t>(amount));
        } else if (keyword == "LAVA") {
            int tx, ty, amount;
            if (iss >> tx >> ty >> amount)
                world.setLava(tx, ty, static_cast<uint8_t>(amount));
        } else if (keyword == "WALL") {
            int tx, ty, wallId;
            if (iss >> tx >> ty >> wallId && wallId >= 0 && wallId <= 55)
                world.setWall(tx, ty, static_cast<TileId>(wallId));
        } else if (keyword == "DOOR_OPEN") {
            int tx, ty;
            if (iss >> tx >> ty)
                world.setDoorOpen(tx, ty, true);
        } else if (keyword == "INVENTORY") {
            int slotIdx, tileId, count;
            if (iss >> slotIdx >> tileId >> count && slotIdx >= 0 &&
                slotIdx < constants::INVENTORY_SLOTS && tileId >= 0 && tileId <= 55) {
                player.getInventory().getSlots()[slotIdx] = {
                    static_cast<TileId>(tileId), count
                };
            }
        } else if (keyword == "SELECTED") {
            int idx;
            if (iss >> idx)
                player.getInventory().selectSlot(idx);
        } else if (keyword == "CHEST") {
            int tx, ty, slotIdx, tileId, count;
            if (iss >> tx >> ty >> slotIdx >> tileId >> count &&
                slotIdx >= 0 && slotIdx < CHEST_SLOTS && tileId >= 0 && tileId <= 55) {
                auto& chest = world.getChest(tx, ty);
                chest[slotIdx] = {static_cast<TileId>(tileId), count};
            }
        } else if (keyword == "END") {
            break;
        }
    }

    int ww = constants::WORLD_WIDTH;
    int wh = constants::WORLD_HEIGHT;
    std::vector<int> heights(ww, 0);
    for (int x = 0; x < ww; ++x) {
        for (int y = 0; y < wh; ++y) {
            if (world.getTile(x, y) != TileId::Air) {
                heights[x] = y;
                break;
            }
        }
    }
    std::vector<Biome> biomes(ww, Biome::Forest);
    world.setBiomeData(biomes, heights);

    return true;
}

bool SaveManager::saveExists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}
