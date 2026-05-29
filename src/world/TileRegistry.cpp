#include "TileRegistry.hpp"

TileRegistry& TileRegistry::instance() {
    static TileRegistry reg;
    return reg;
}

TileRegistry::TileRegistry() {
    m_tiles[TileId::Air]          = {TileId::Air,          "Air",          false, 0, BLANK};
    m_tiles[TileId::Grass]        = {TileId::Grass,        "Grass",        true,  1, Color{80, 180, 60, 255}};
    m_tiles[TileId::Dirt]         = {TileId::Dirt,         "Dirt",         true,  1, Color{140, 100, 60, 255}};
    m_tiles[TileId::Stone]        = {TileId::Stone,        "Stone",        true,  3, Color{120, 120, 120, 255}};
    m_tiles[TileId::Leaf]         = {TileId::Leaf,         "Leaf",         false, 1, Color{40, 140, 40, 255}};
    m_tiles[TileId::Wood]         = {TileId::Wood,         "Wood",         false, 2, Color{100, 70, 40, 255}};
    m_tiles[TileId::Sand]         = {TileId::Sand,         "Sand",         true,  1, Color{210, 190, 140, 255}};
    m_tiles[TileId::SnowBlock]    = {TileId::SnowBlock,    "SnowBlock",    true,  1, Color{230, 240, 250, 255}};
    m_tiles[TileId::Ice]          = {TileId::Ice,          "Ice",          true,  2, Color{180, 220, 240, 255}};
    m_tiles[TileId::Clay]         = {TileId::Clay,         "Clay",         true,  1, Color{170, 130, 100, 255}};
    m_tiles[TileId::Gravel]       = {TileId::Gravel,       "Gravel",       true,  2, Color{150, 140, 130, 255}};
    m_tiles[TileId::CopperOre]    = {TileId::CopperOre,    "CopperOre",    true,  4, Color{200, 120, 60, 255}};
    m_tiles[TileId::IronOre]      = {TileId::IronOre,      "IronOre",      true,  5, Color{200, 170, 120, 255}};
    m_tiles[TileId::GoldOre]      = {TileId::GoldOre,      "GoldOre",      true,  6, Color{220, 200, 40, 255}};
    m_tiles[TileId::Cactus]       = {TileId::Cactus,       "Cactus",       true,  1, Color{50, 170, 50, 255}};
    m_tiles[TileId::MushroomGrass] = {TileId::MushroomGrass, "MushroomGrass", true, 1, Color{100, 60, 140, 255}};
    m_tiles[TileId::Marble]       = {TileId::Marble,       "Marble",       true,  3, Color{200, 200, 210, 255}};
    m_tiles[TileId::Granite]      = {TileId::Granite,      "Granite",      true,  4, Color{80, 70, 90, 255}};
    m_tiles[TileId::Mud]          = {TileId::Mud,          "Mud",          true,  1, Color{100, 80, 50, 255}};
    m_tiles[TileId::JungleGrass]  = {TileId::JungleGrass,  "JungleGrass",  true,  1, Color{60, 140, 40, 255}};
    m_tiles[TileId::Hellstone]    = {TileId::Hellstone,    "Hellstone",    true,  7, Color{180, 60, 30, 255}};
    m_tiles[TileId::Pickaxe]      = {TileId::Pickaxe,      "Pickaxe",      false, 0, Color{160, 130, 100, 255}};
    m_tiles[TileId::Axe]          = {TileId::Axe,          "Axe",          false, 0, Color{180, 160, 140, 255}};
    m_tiles[TileId::Sword]        = {TileId::Sword,        "Sword",        false, 0, Color{200, 200, 210, 255}};
    m_tiles[TileId::Planks]       = {TileId::Planks,       "Planks",       true,  2, Color{160, 120, 80, 255}};
    m_tiles[TileId::Torch]        = {TileId::Torch,        "Torch",        false, 0, Color{255, 200, 50, 255}};
    m_tiles[TileId::DirtWall]     = {TileId::DirtWall,     "Dirt Wall",    false, 1, Color{140, 100, 60, 200}};
    m_tiles[TileId::StoneWall]    = {TileId::StoneWall,    "Stone Wall",   false, 1, Color{120, 120, 120, 200}};
    m_tiles[TileId::WoodWall]     = {TileId::WoodWall,     "Wood Wall",    false, 1, Color{100, 70, 40, 200}};
    m_tiles[TileId::PlanksWall]   = {TileId::PlanksWall,   "Planks Wall",  false, 1, Color{160, 120, 80, 200}};
    m_tiles[TileId::Water]        = {TileId::Water,        "Water",        false, 0, Color{30, 100, 200, 150}};
    m_tiles[TileId::Lava]         = {TileId::Lava,         "Lava",         false, 0, Color{255, 80, 0, 200}};
}

const TileDefinition& TileRegistry::get(TileId id) const {
    return m_tiles.at(id);
}

TileId TileRegistry::getIdByName(const std::string& name) const {
    for (const auto& [id, def] : m_tiles) {
        if (def.name == name)
            return id;
    }
    return TileId::Air;
}
