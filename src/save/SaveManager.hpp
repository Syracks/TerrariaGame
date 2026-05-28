#pragma once

#include <string>
#include <array>
#include "core/Constants.hpp"

class World;
class Player;

struct SlotInfo {
    std::string name;
    WorldSize size = WorldSize::Medium;
    unsigned int seed = 0;
    bool occupied = false;
};

class SaveManager {
public:
    static constexpr int SLOT_COUNT = 5;

    static std::string getSlotPath(int slot);
    static std::string getSlotMetaPath(int slot);
    static std::string getSlotDataPath(int slot);

    static SlotInfo getSlotInfo(int slot);
    static bool saveSlotMeta(int slot, const SlotInfo& info);
    static bool saveSlot(int slot, const World& world, const Player& player,
                         const std::string& name, WorldSize size, unsigned int seed);
    static bool loadSlot(int slot, World& world, Player& player);
    static void deleteSlot(int slot);

    static std::array<SlotInfo, SLOT_COUNT> listSlots();

    static bool save(const World& world, const Player& player, const std::string& filepath);
    static bool load(World& world, Player& player, const std::string& filepath);
    static bool saveExists(const std::string& filepath);
};
