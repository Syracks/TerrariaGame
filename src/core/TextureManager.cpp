#include "TextureManager.hpp"

TextureManager& TextureManager::instance() {
    static TextureManager mgr;
    return mgr;
}

TextureManager::~TextureManager() {
}

void TextureManager::unloadAll() {
    for (auto& [id, tex] : m_textures) {
        if (tex.id > 0) {
            UnloadTexture(tex);
        }
    }
    m_textures.clear();
    if (m_doorOpenTexture.id > 0) {
        UnloadTexture(m_doorOpenTexture);
        m_doorOpenTexture = {};
    }
}

bool TextureManager::loadAll() {
    unloadAll();

    loadTileTexture("dirt", TileId::Dirt);
    loadTileTexture("grass", TileId::Grass);
    loadTileTexture("stone", TileId::Stone);
    loadTileTexture("wood", TileId::Wood);
    loadTileTexture("leaves", TileId::Leaf);
    loadTileTexture("sand", TileId::Sand);
    loadTileTexture("snow", TileId::SnowBlock);
    loadTileTexture("ice", TileId::Ice);
    loadTileTexture("clay", TileId::Clay);
    loadTileTexture("gravel", TileId::Gravel);
    loadTileTexture("copper", TileId::CopperOre);
    loadTileTexture("iron", TileId::IronOre);
    loadTileTexture("gold", TileId::GoldOre);
    loadTileTexture("cactus", TileId::Cactus);
    loadTileTexture("mushroom", TileId::MushroomGrass);
    loadTileTexture("marble", TileId::Marble);
    loadTileTexture("granite", TileId::Granite);
    loadTileTexture("mud", TileId::Mud);
    loadTileTexture("junglegrass", TileId::JungleGrass);
    loadTileTexture("hellstone", TileId::Hellstone);
    loadTileTexture("pickaxe", TileId::Pickaxe);
    loadTileTexture("axe", TileId::Axe);
    loadTileTexture("sword", TileId::Sword);
    loadTileTexture("planks", TileId::Planks);
    loadTileTexture("torch", TileId::Torch);
    loadTileTexture("dirtwall", TileId::DirtWall);
    loadTileTexture("stonewall", TileId::StoneWall);
    loadTileTexture("plankswall", TileId::PlanksWall);
    loadTileTexture("workbench", TileId::Workbench);
    loadTileTexture("furnace", TileId::Furnace);
    loadTileTexture("anvil", TileId::Anvil);
    loadTileTexture("chest", TileId::ChestBlock);
    loadTileTexture("copperbar", TileId::CopperBar);
    loadTileTexture("ironbar", TileId::IronBar);
    loadTileTexture("goldbar", TileId::GoldBar);
    loadTileTexture("gel", TileId::Gel);
    loadToolTexture("iron_axe", TileId::IronAxe);
    loadToolTexture("iron_pickaxe", TileId::IronPickaxe);
    loadToolTexture("iron_sword", TileId::IronSword);
    loadToolTexture("copper_pickaxe", TileId::CopperPickaxe);
    loadToolTexture("copper_axe", TileId::CopperAxe);
    loadToolTexture("copper_sword", TileId::CopperSword);
    loadToolTexture("gold_pickaxe", TileId::GoldPickaxe);
    loadToolTexture("gold_axe", TileId::GoldAxe);
    loadToolTexture("gold_sword", TileId::GoldSword);
    loadToolTexture("copper_hammer", TileId::CopperHammer);
    loadToolTexture("iron_hammer", TileId::IronHammer);
    loadToolTexture("gold_hammer", TileId::GoldHammer);
    loadTileTexture("hammer", TileId::Hammer);
    loadTileTexture("door_closed", TileId::Door);
    {
        std::string path = "assets/textures/tiles/door_opened_0.png";
        if (FileExists(path.c_str())) {
            m_doorOpenTexture = LoadTexture(path.c_str());
        }
    }
    loadTileTexture("wooden_chair", TileId::WoodenChair);
    loadTileTexture("wooden_table", TileId::WoodenTable);

    return true;
}

void TextureManager::loadTileTexture(const std::string& tileName, TileId id) {
    std::string path = "assets/textures/tiles/" + tileName + "_0.png";
    if (FileExists(path.c_str())) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id > 0) {
            m_textures[id] = tex;
        }
    }
}

void TextureManager::loadToolTexture(const std::string& fileName, TileId id) {
    std::string path = "assets/textures/tools/" + fileName + ".png";
    if (FileExists(path.c_str())) {
        Texture2D tex = LoadTexture(path.c_str());
        if (tex.id > 0) {
            m_textures[id] = tex;
        }
    }
}

const Texture2D& TextureManager::getTexture(TileId id) const {
    static Texture2D empty{};
    auto it = m_textures.find(id);
    if (it == m_textures.end()) {
        return empty;
    }
    return it->second;
}
