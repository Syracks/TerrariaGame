#pragma once

#include <raylib.h>
#include <string>
#include <cstdint>

enum class TileId : uint8_t {
    Air = 0,
    Grass = 1,
    Dirt = 2,
    Stone = 3,
    Wood = 4,
    Leaf = 5,
    Sand = 6,
    SnowBlock = 7,
    Ice = 8,
    Clay = 9,
    Gravel = 10,
    CopperOre = 11,
    IronOre = 12,
    GoldOre = 13,
    Cactus = 14,
    MushroomGrass = 15,
    Marble = 16,
    Granite = 17,
    Mud = 18,
    JungleGrass = 19,
    Hellstone = 20,
    Pickaxe = 21,
    Axe = 22,
    Sword = 23,
    Planks = 24,
    Torch = 25,
    DirtWall = 26,
    StoneWall = 27,
    WoodWall = 28,
    PlanksWall = 29,
    Water = 30,
    Lava = 31,
    Workbench = 32,
    Furnace = 33,
    Anvil = 34,
    ChestBlock = 35,
    CopperBar = 36,
    IronBar = 37,
    GoldBar = 38,
    Gel = 39,
    CopperPickaxe = 40,
    IronPickaxe = 41,
    GoldPickaxe = 42,
    CopperAxe = 43,
    IronAxe = 44,
    GoldAxe = 45,
    CopperSword = 46,
    IronSword = 47,
    GoldSword = 48,
    Hammer = 49,
    CopperHammer = 50,
    IronHammer = 51,
    GoldHammer = 52,
    Door = 53,
    WoodenChair = 54,
    WoodenTable = 55,
    CopperBow = 56,
    IronBow = 57,
    GoldBow = 58,
    Arrow = 59,
    LeafProjectile = 60,
    Fireball = 61,
    AncientSeed = 62
};

constexpr int MAX_LIQUID_LEVEL = 255;

enum class Biome { Forest, Desert, Snow, Plains, Jungle, Ocean, Beach };

struct LiquidInfo {
    uint8_t amount = 0;
    bool isLava = false;
};

struct TileDefinition {
    TileId id;
    std::string name;
    bool solid;
    int hardness;
    Color color;
};
