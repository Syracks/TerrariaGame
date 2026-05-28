#pragma once

#include "world/Tile.hpp"
#include <raylib.h>
#include <unordered_map>

class TextureManager {
public:
    static TextureManager& instance();

    bool loadAll();
    void unloadAll();

    const Texture2D& getTexture(TileId id) const;

private:
    TextureManager() = default;
    ~TextureManager();
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    void loadTileTexture(const std::string& tileName, TileId id);

    std::unordered_map<TileId, Texture2D> m_textures;
};
