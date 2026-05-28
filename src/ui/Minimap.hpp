#pragma once

#include <raylib.h>

class World;

class Minimap {
public:
    Minimap() = default;
    ~Minimap();

    void rebuild(const World& world);
    void render(const World& world, Vector2 playerWorldPos, bool visible);
    void markDirty() { m_dirty = true; }

private:
    Texture2D m_texture{};
    bool m_dirty = true;
    int m_updateCounter = 0;

    static Color getTileColor(const World& world, int tx, int ty);
};
