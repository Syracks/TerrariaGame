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
#include <cstdlib>

static void ensureDir(const std::string& path) {
    mkdir(path.c_str(), 0755);
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
    info.seed = static_cast<unsigned int>(std::stoul(seedStr));
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
    return true;
}

bool SaveManager::saveSlot(int slot, const World& world, const Player& player,
                           const std::string& name, WorldSize size, unsigned int seed) {
    ensureDir("saves");
    ensureDir(getSlotPath(slot));
    SlotInfo info;
    info.name = name;
    info.size = size;
    info.seed = seed;
    info.occupied = true;
    if (!saveSlotMeta(slot, info))
        return false;
    return save(world, player, getSlotDataPath(slot));
}

bool SaveManager::loadSlot(int slot, World& world, Player& player) {
    return load(world, player, getSlotDataPath(slot));
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

bool SaveManager::save(const World& world, const Player& player, const std::string& filepath) {
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Failed to open save file for writing: " << filepath << std::endl;
        return false;
    }

    file << "WORLD " << world.getWorldWidth() << " " << world.getWorldHeight() << "\n";
    file << "PLAYER " << player.getPosition().x << " " << player.getPosition().y << "\n";

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
    file << "END\n";
    return true;
}

bool SaveManager::load(World& world, Player& player, const std::string& filepath) {
    std::ifstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "No save file found: " << filepath << std::endl;
        return false;
    }

    world.clear();
    std::string line;

    while (std::getline(file, line)) {
        std::istringstream iss(line);
        std::string keyword;
        iss >> keyword;

        if (keyword == "WORLD") {
            int w, h;
            iss >> w >> h;
        } else if (keyword == "PLAYER") {
            float x, y;
            iss >> x >> y;
            player.setPosition({x, y});
            player.setVelocity({0, 0});
        } else if (keyword == "TILE") {
            int tx, ty, tileId;
            iss >> tx >> ty >> tileId;
            world.setTile(tx, ty, static_cast<TileId>(tileId));
        } else if (keyword == "WATER") {
            int tx, ty, amount;
            iss >> tx >> ty >> amount;
            world.setWater(tx, ty, static_cast<uint8_t>(amount));
        } else if (keyword == "LAVA") {
            int tx, ty, amount;
            iss >> tx >> ty >> amount;
            world.setLava(tx, ty, static_cast<uint8_t>(amount));
        } else if (keyword == "WALL") {
            int tx, ty, wallId;
            iss >> tx >> ty >> wallId;
            world.setWall(tx, ty, static_cast<TileId>(wallId));
        } else if (keyword == "INVENTORY") {
            int slotIdx, tileId, count;
            iss >> slotIdx >> tileId >> count;
            if (slotIdx >= 0 && slotIdx < constants::INVENTORY_SLOTS) {
                player.getInventory().getSlots()[slotIdx] = {
                    static_cast<TileId>(tileId), count
                };
            }
        } else if (keyword == "SELECTED") {
            int idx;
            iss >> idx;
            player.getInventory().selectSlot(idx);
        } else if (keyword == "END") {
            break;
        }
    }

    return true;
}

bool SaveManager::saveExists(const std::string& filepath) {
    std::ifstream file(filepath);
    return file.good();
}
