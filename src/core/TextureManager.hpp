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
    const Texture2D& getDoorOpenTexture() const { return m_doorOpenTexture; }

private:
    TextureManager() = default;
    ~TextureManager();
    TextureManager(const TextureManager&) = delete;
    TextureManager& operator=(const TextureManager&) = delete;

    void loadTileTexture(const std::string& tileName, TileId id);
    void loadToolTexture(const std::string& fileName, TileId id);

    std::unordered_map<TileId, Texture2D> m_textures;
    Texture2D m_doorOpenTexture{};
};
