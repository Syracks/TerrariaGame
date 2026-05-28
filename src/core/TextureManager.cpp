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
}

bool TextureManager::loadAll() {
    unloadAll();

    loadTileTexture("dirt", TileId::Dirt);
    loadTileTexture("grass", TileId::Grass);
    loadTileTexture("stone", TileId::Stone);
    loadTileTexture("wood", TileId::Wood);
    loadTileTexture("leaf", TileId::Leaf);
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
    loadTileTexture("woodwall", TileId::WoodWall);
    loadTileTexture("plankswall", TileId::PlanksWall);

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

const Texture2D& TextureManager::getTexture(TileId id) const {
    static Texture2D empty{};
    auto it = m_textures.find(id);
    if (it == m_textures.end()) {
        return empty;
    }
    return it->second;
}
