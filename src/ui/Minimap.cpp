#include "Minimap.hpp"
#include "world/World.hpp"
#include "world/Tile.hpp"
#include "world/TileRegistry.hpp"
#include "core/Constants.hpp"

#include <cmath>

namespace {
    constexpr int MM_WIDTH = 200;
    constexpr int MM_HEIGHT = 140;
    constexpr int MARGIN = 10;
}

Minimap::~Minimap() {
    if (m_texture.id > 0) {
        UnloadTexture(m_texture);
        m_texture = {};
    }
}

Color Minimap::getTileColor(const World& world, int tx, int ty) {
    TileId id = world.getTile(tx, ty);
    if (id == TileId::Air) {
        TileId wall = world.getWall(tx, ty);
        if (wall != TileId::Air) {
            return TileRegistry::instance().get(wall).color;
        }
        return Color{0, 0, 0, 0};
    }
    switch (id) {
        case TileId::Grass:
        case TileId::JungleGrass:
        case TileId::MushroomGrass:
            return Color{60, 160, 40, 255};
        case TileId::Dirt:
        case TileId::Mud:
            return Color{130, 90, 50, 255};
        case TileId::Stone:
            return Color{100, 100, 100, 255};
        case TileId::Sand:
            return Color{200, 180, 130, 255};
        case TileId::SnowBlock:
            return Color{200, 210, 230, 255};
        case TileId::Ice:
            return Color{150, 200, 230, 255};
        case TileId::Clay:
            return Color{160, 120, 90, 255};
        case TileId::Gravel:
            return Color{140, 130, 120, 255};
        case TileId::CopperOre:
            return Color{200, 120, 60, 255};
        case TileId::IronOre:
            return Color{180, 150, 100, 255};
        case TileId::GoldOre:
            return Color{220, 200, 40, 255};
        case TileId::Cactus:
            return Color{50, 150, 50, 255};
        case TileId::Marble:
            return Color{190, 190, 200, 255};
        case TileId::Granite:
            return Color{80, 70, 90, 255};
        case TileId::Hellstone:
            return Color{160, 50, 20, 255};
        case TileId::Wood:
        case TileId::Leaf:
            return Color{40, 120, 40, 255};
        case TileId::Planks:
            return Color{150, 110, 70, 255};
        case TileId::Torch:
            return Color{255, 200, 50, 255};
        default:
            return Color{100, 100, 100, 255};
    }
}

void Minimap::rebuild(const World& world) {
    int w = world.getWorldWidth();
    int h = world.getWorldHeight();
    if (w <= 0 || h <= 0) return;

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    int imgW, imgH;
    if (aspect > static_cast<float>(MM_WIDTH) / static_cast<float>(MM_HEIGHT)) {
        imgW = MM_WIDTH;
        imgH = std::max(1, static_cast<int>(MM_WIDTH / aspect));
    } else {
        imgH = MM_HEIGHT;
        imgW = std::max(1, static_cast<int>(MM_HEIGHT * aspect));
    }

    Image img = GenImageColor(imgW, imgH, Color{0, 0, 0, 200});

    float scaleX = static_cast<float>(imgW) / static_cast<float>(w);
    float scaleY = static_cast<float>(imgH) / static_cast<float>(h);

    for (int py = 0; py < imgH; ++py) {
        for (int px = 0; px < imgW; ++px) {
            int tx = static_cast<int>(px / scaleX);
            int ty = static_cast<int>(py / scaleY);
            tx = std::max(0, std::min(w - 1, tx));
            ty = std::max(0, std::min(h - 1, ty));
            Color c = getTileColor(world, tx, ty);
            if (c.a > 0) {
                ImageDrawPixel(&img, px, py, c);
            }
        }
    }

    if (m_texture.id > 0) {
        UnloadTexture(m_texture);
    }
    m_texture = LoadTextureFromImage(img);
    UnloadImage(img);
    m_dirty = false;
}

void Minimap::render(const World& world, Vector2 playerWorldPos, bool visible) {
    if (!visible) return;

    m_updateCounter++;
    if (m_dirty && m_updateCounter % 30 == 0) {
        rebuild(world);
    }

    if (m_texture.id <= 0) return;

    int w = world.getWorldWidth();
    int h = world.getWorldHeight();
    if (w <= 0 || h <= 0) return;

    float aspect = static_cast<float>(w) / static_cast<float>(h);
    int mmW, mmH;
    if (aspect > static_cast<float>(MM_WIDTH) / static_cast<float>(MM_HEIGHT)) {
        mmW = MM_WIDTH;
        mmH = std::max(1, static_cast<int>(MM_WIDTH / aspect));
    } else {
        mmH = MM_HEIGHT;
        mmW = std::max(1, static_cast<int>(MM_HEIGHT * aspect));
    }

    int mmX = constants::SCREEN_WIDTH - mmW - MARGIN;
    int mmY = MARGIN + 45;

    float texScale = static_cast<float>(mmW) / m_texture.width;

    DrawRectangle(mmX - 2, mmY - 2, mmW + 4, mmH + 4, Color{60, 60, 80, 200});
    DrawRectangleLines(mmX - 2, mmY - 2, mmW + 4, mmH + 4, Color{100, 100, 140, 255});

    DrawTextureEx(m_texture, {static_cast<float>(mmX), static_cast<float>(mmY)},
                  0.0f, texScale, WHITE);

    float playerMX = mmX + (playerWorldPos.x / constants::TILE_SIZE) *
                   (static_cast<float>(mmW) / static_cast<float>(w));
    float playerMY = mmY + (playerWorldPos.y / constants::TILE_SIZE) *
                   (static_cast<float>(mmH) / static_cast<float>(h));

    DrawCircle(static_cast<int>(playerMX), static_cast<int>(playerMY), 3, WHITE);
    DrawCircle(static_cast<int>(playerMX), static_cast<int>(playerMY), 2, Color{255, 255, 100, 255});
}
