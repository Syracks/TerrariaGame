#pragma once

#include "Tile.hpp"
#include <unordered_map>

class TileRegistry {
public:
    static TileRegistry& instance();

    const TileDefinition& get(TileId id) const;
    TileId getIdByName(const std::string& name) const;

private:
    TileRegistry();
    std::unordered_map<TileId, TileDefinition> m_tiles;
};
