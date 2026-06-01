#include "ItemDefinition.hpp"

ItemDatabase& ItemDatabase::instance() {
    static ItemDatabase db;
    return db;
}

ItemDatabase::ItemDatabase() {
    auto def = [](TileId id, const std::string& name, int max, bool placeable, bool consumable, ToolStats tool) {
        return ItemDefinition{id, name, max, placeable, consumable, tool};
    };
    auto tool = [](float speed, int level, int dmg) { return ToolStats{speed, level, dmg}; };

    m_items[TileId::Dirt]          = def(TileId::Dirt,          "Dirt",          999, true,  false, tool(0,0,0));
    m_items[TileId::Grass]         = def(TileId::Grass,         "Grass",         999, true,  false, tool(0,0,0));
    m_items[TileId::Stone]         = def(TileId::Stone,         "Stone",         999, true,  false, tool(0,0,0));
    m_items[TileId::Wood]          = def(TileId::Wood,          "Wood",          999, true,  false, tool(0,0,0));
    m_items[TileId::Leaf]          = def(TileId::Leaf,          "Leaf",          999, true,  false, tool(0,0,0));
    m_items[TileId::Sand]          = def(TileId::Sand,          "Sand",          999, true,  false, tool(0,0,0));
    m_items[TileId::SnowBlock]     = def(TileId::SnowBlock,     "Snow Block",    999, true,  false, tool(0,0,0));
    m_items[TileId::Ice]           = def(TileId::Ice,           "Ice",           999, true,  false, tool(0,0,0));
    m_items[TileId::Clay]          = def(TileId::Clay,          "Clay",          999, true,  false, tool(0,0,0));
    m_items[TileId::Gravel]        = def(TileId::Gravel,        "Gravel",        999, true,  false, tool(0,0,0));
    m_items[TileId::CopperOre]     = def(TileId::CopperOre,     "Copper Ore",    999, true,  false, tool(0,0,0));
    m_items[TileId::IronOre]       = def(TileId::IronOre,       "Iron Ore",      999, true,  false, tool(0,0,0));
    m_items[TileId::GoldOre]       = def(TileId::GoldOre,       "Gold Ore",      999, true,  false, tool(0,0,0));
    m_items[TileId::Cactus]        = def(TileId::Cactus,        "Cactus",        999, true,  false, tool(0,0,0));
    m_items[TileId::MushroomGrass] = def(TileId::MushroomGrass, "Mushroom Grass",999, true,  false, tool(0,0,0));
    m_items[TileId::Marble]        = def(TileId::Marble,        "Marble",        999, true,  false, tool(0,0,0));
    m_items[TileId::Granite]       = def(TileId::Granite,       "Granite",       999, true,  false, tool(0,0,0));
    m_items[TileId::Mud]           = def(TileId::Mud,           "Mud",           999, true,  false, tool(0,0,0));
    m_items[TileId::JungleGrass]   = def(TileId::JungleGrass,   "Jungle Grass",  999, true,  false, tool(0,0,0));
    m_items[TileId::Hellstone]     = def(TileId::Hellstone,     "Hellstone",     999, true,  false, tool(0,0,0));
    m_items[TileId::Planks]        = def(TileId::Planks,        "Planks",        999, true,  false, tool(0,0,0));
    m_items[TileId::Torch]         = def(TileId::Torch,         "Torch",          99, true,  false, tool(0,0,0));
    m_items[TileId::DirtWall]      = def(TileId::DirtWall,      "Dirt Wall",     999, true,  false, tool(0,0,0));
    m_items[TileId::StoneWall]     = def(TileId::StoneWall,     "Stone Wall",    999, true,  false, tool(0,0,0));
    m_items[TileId::WoodWall]      = def(TileId::WoodWall,      "Wood Wall",     999, true,  false, tool(0,0,0));
    m_items[TileId::PlanksWall]    = def(TileId::PlanksWall,    "Planks Wall",   999, true,  false, tool(0,0,0));
    m_items[TileId::CopperBar]     = def(TileId::CopperBar,     "Copper Bar",    999, false, true, tool(0,0,0));
    m_items[TileId::IronBar]       = def(TileId::IronBar,       "Iron Bar",      999, false, true, tool(0,0,0));
    m_items[TileId::GoldBar]       = def(TileId::GoldBar,       "Gold Bar",      999, false, true, tool(0,0,0));
    m_items[TileId::Gel]           = def(TileId::Gel,           "Gel",           999, false, true, tool(0,0,0));
    m_items[TileId::Water]         = def(TileId::Water,         "Water",         999, false, false, tool(0,0,0));
    m_items[TileId::Lava]          = def(TileId::Lava,          "Lava",          999, false, false, tool(0,0,0));
    m_items[TileId::Workbench]     = def(TileId::Workbench,     "Workbench",       1, true,  false, tool(0,0,0));
    m_items[TileId::Furnace]       = def(TileId::Furnace,       "Furnace",         1, true,  false, tool(0,0,0));
    m_items[TileId::Anvil]         = def(TileId::Anvil,         "Anvil",           1, true,  false, tool(0,0,0));
    m_items[TileId::ChestBlock]    = def(TileId::ChestBlock,    "Chest",           1, true,  false, tool(0,0,0));

    m_items[TileId::Pickaxe] = def(TileId::Pickaxe, "Wood Pickaxe", 1, false, false, tool(1.0f, 0, 4));
    m_items[TileId::Axe]     = def(TileId::Axe,     "Wood Axe",     1, false, false, tool(1.0f, 0, 4));
    m_items[TileId::Sword]   = def(TileId::Sword,   "Wood Sword",   1, false, false, tool(0,    0, 15));
    m_items[TileId::CopperPickaxe] = def(TileId::CopperPickaxe, "Copper Pickaxe", 1, false, false, tool(1.2f, 1, 6));
    m_items[TileId::CopperAxe]     = def(TileId::CopperAxe,     "Copper Axe",     1, false, false, tool(1.2f, 1, 6));
    m_items[TileId::CopperSword]   = def(TileId::CopperSword,   "Copper Sword",   1, false, false, tool(0,    1, 22));
    m_items[TileId::IronPickaxe]   = def(TileId::IronPickaxe,   "Iron Pickaxe",   1, false, false, tool(1.5f, 2, 8));
    m_items[TileId::IronAxe]       = def(TileId::IronAxe,       "Iron Axe",       1, false, false, tool(1.5f, 2, 8));
    m_items[TileId::IronSword]     = def(TileId::IronSword,     "Iron Sword",     1, false, false, tool(0,    2, 25));
    m_items[TileId::GoldPickaxe]   = def(TileId::GoldPickaxe,   "Gold Pickaxe",   1, false, false, tool(1.8f, 3, 10));
    m_items[TileId::GoldAxe]       = def(TileId::GoldAxe,       "Gold Axe",       1, false, false, tool(1.8f, 3, 10));
    m_items[TileId::GoldSword]     = def(TileId::GoldSword,     "Gold Sword",     1, false, false, tool(0,    3, 30));
    m_items[TileId::Hammer] = def(TileId::Hammer, "Wood Hammer", 1, false, false, tool(1.0f, 0, 4));
    m_items[TileId::CopperHammer] = def(TileId::CopperHammer, "Copper Hammer", 1, false, false, tool(1.2f, 1, 6));
    m_items[TileId::IronHammer]   = def(TileId::IronHammer,   "Iron Hammer",   1, false, false, tool(1.5f, 2, 8));
    m_items[TileId::GoldHammer]   = def(TileId::GoldHammer,   "Gold Hammer",   1, false, false, tool(1.8f, 3, 10));
    m_items[TileId::Door]          = def(TileId::Door,          "Door",          1, true,  false, tool(0,0,0));
    m_items[TileId::WoodenChair]   = def(TileId::WoodenChair,   "Wooden Chair",  1, true,  false, tool(0,0,0));
    m_items[TileId::WoodenTable]   = def(TileId::WoodenTable,   "Wooden Table",  1, true,  false, tool(0,0,0));
}

const ItemDefinition& ItemDatabase::get(TileId id) const {
    static ItemDefinition fallback;
    auto it = m_items.find(id);
    if (it == m_items.end()) return fallback;
    return it->second;
}

int ItemDatabase::getMaxStack(TileId id) const {
    return get(id).maxStack;
}
