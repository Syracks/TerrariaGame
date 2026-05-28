#pragma once

#include <raylib.h>
#include <string>

enum class TileId : int {
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
    PlanksWall = 29
};

struct TileDefinition {
    TileId id;
    std::string name;
    bool solid;
    int hardness;
    Color color;
};
