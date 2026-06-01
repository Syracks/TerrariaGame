#include "RenderSystem.hpp"
#include "world/World.hpp"
#include "world/Chunk.hpp"
#include "world/Tile.hpp"
#include "world/TileRegistry.hpp"
#include "core/Constants.hpp"
#include "core/Math.hpp"
#include "core/TextureManager.hpp"

#include <vector>
#include <cmath>
#include <algorithm>

namespace {
    constexpr float TORCH_RADIUS = 9.0f * constants::TILE_SIZE;
    constexpr float PLAYER_LIGHT_RADIUS = 7.0f * constants::TILE_SIZE;
    constexpr float SURFACE_LIGHT = 1.0f;
    constexpr float UNDERGROUND_LIGHT = 0.08f;
    constexpr float DEPTH_FALLOFF = 28.0f;
    constexpr unsigned char MAX_DARKNESS = 205;

    struct LightSrc {
        float x;
        float y;
        float radius;
        float intensity;
    };

    float clamp01(float v) {
        return std::max(0.0f, std::min(1.0f, v));
    }

    float smooth01(float t) {
        t = clamp01(t);
        return t * t * (3.0f - 2.0f * t);
    }
}

static void drawTileOverlay(int x, int y, Color color, int height = 3) {
    DrawRectangle(x, y, constants::TILE_SIZE, height, color);
}

void RenderSystem::renderWorld(const World& world, const Camera2D& camera) {
    float viewLeft   = camera.target.x - camera.offset.x;
    float viewTop    = camera.target.y - camera.offset.y;
    float viewRight  = viewLeft + constants::SCREEN_WIDTH;
    float viewBottom = viewTop + constants::SCREEN_HEIGHT;

    int startTileX = std::max(0, math::worldToTileX(viewLeft) - 1);
    int startTileY = std::max(0, math::worldToTileY(viewTop) - 1);
    int endTileX   = std::min(constants::WORLD_WIDTH - 1, math::worldToTileX(viewRight) + 1);
    int endTileY   = std::min(constants::WORLD_HEIGHT - 1, math::worldToTileY(viewBottom) + 1);

    auto& registry = TileRegistry::instance();
    auto& texMgr = TextureManager::instance();

    for (int tileX = startTileX; tileX <= endTileX; ++tileX) {
        for (int tileY = startTileY; tileY <= endTileY; ++tileY) {
            TileId wall = world.getWall(tileX, tileY);
            if (wall != TileId::Air) {
                int px = static_cast<int>(math::tileToWorldX(tileX));
                int py = static_cast<int>(math::tileToWorldY(tileY));
                TileId foreground = world.getTile(tileX, tileY);
                unsigned char wallAlpha = (foreground == TileId::Air ||
                    !world.isSolid(tileX, tileY) || foreground == TileId::Door) ? 200 : 40;
                const Texture2D& wtex = texMgr.getTexture(wall);
                if (wtex.id > 0) {
                    DrawTexture(wtex, px, py, Color{255, 255, 255, wallAlpha});
                } else {
                    const auto& wdef = registry.get(wall);
                    DrawRectangle(px, py, constants::TILE_SIZE, constants::TILE_SIZE,
                                  Color{wdef.color.r, wdef.color.g, wdef.color.b, wallAlpha});
                }
            }
        }
    }

    for (int tileX = startTileX; tileX <= endTileX; ++tileX) {
        for (int tileY = startTileY; tileY <= endTileY; ++tileY) {
            TileId id = world.getTile(tileX, tileY);
            if (id == TileId::Air)
                continue;

            int px = static_cast<int>(math::tileToWorldX(tileX));
            int py = static_cast<int>(math::tileToWorldY(tileY));

            const Texture2D& tex = (id == TileId::Door && world.isDoorOpen(tileX, tileY))
                ? texMgr.getDoorOpenTexture()
                : texMgr.getTexture(id);
            if (tex.id > 0) {
                if (id == TileId::Door && tex.height > constants::TILE_SIZE) {
                    Rectangle source = {0, 0, static_cast<float>(constants::TILE_SIZE), static_cast<float>(constants::TILE_SIZE)};
                    if (world.isInBounds(tileX, tileY + 1) && world.getTile(tileX, tileY + 1) == TileId::Door) {
                        source.y = 0.0f;
                    } else if (world.isInBounds(tileX, tileY - 1) && world.getTile(tileX, tileY - 1) == TileId::Door) {
                        source.y = static_cast<float>(constants::TILE_SIZE);
                    }
                    DrawTextureRec(tex, source, {static_cast<float>(px), static_cast<float>(py)}, WHITE);
                } else if (id == TileId::WoodenTable && tex.width > constants::TILE_SIZE) {
                    Rectangle source = {0, 0, static_cast<float>(constants::TILE_SIZE), static_cast<float>(constants::TILE_SIZE)};
                    bool right = world.isInBounds(tileX+1, tileY) && world.getTile(tileX+1, tileY) == TileId::WoodenTable;
                    bool left  = world.isInBounds(tileX-1, tileY) && world.getTile(tileX-1, tileY) == TileId::WoodenTable;
                    bool down  = world.isInBounds(tileX, tileY+1) && world.getTile(tileX, tileY+1) == TileId::WoodenTable;
                    bool up    = world.isInBounds(tileX, tileY-1) && world.getTile(tileX, tileY-1) == TileId::WoodenTable;
                    if (right && down) {
                        source.x = 0; source.y = 0;
                    } else if (left && down) {
                        source.x = static_cast<float>(constants::TILE_SIZE); source.y = 0;
                    } else if (right && up) {
                        source.x = 0; source.y = static_cast<float>(constants::TILE_SIZE);
                    } else if (left && up) {
                        source.x = static_cast<float>(constants::TILE_SIZE); source.y = static_cast<float>(constants::TILE_SIZE);
                    }
                    DrawTextureRec(tex, source, {static_cast<float>(px), static_cast<float>(py)}, WHITE);
                } else {
                    DrawTexture(tex, px, py, WHITE);
                }
            } else {
                const auto& def = registry.get(id);
                DrawRectangle(px, py, constants::TILE_SIZE, constants::TILE_SIZE, def.color);
            }

            switch (id) {
                case TileId::Grass:
                    drawTileOverlay(px, py, Color{50, 140, 40, 255}, 4);
                    break;
                case TileId::MushroomGrass:
                    drawTileOverlay(px, py, Color{130, 40, 180, 255}, 4);
                    break;
                case TileId::SnowBlock:
                    drawTileOverlay(px, py, Color{240, 248, 255, 255}, 2);
                    break;
                case TileId::Sand:
                    drawTileOverlay(px, py, Color{220, 200, 150, 255}, 1);
                    break;
                case TileId::Ice:
                    DrawRectangle(px, py, constants::TILE_SIZE, constants::TILE_SIZE,
                                  Color{180, 220, 240, 100});
                    break;
                case TileId::CopperOre:
                case TileId::IronOre:
                case TileId::GoldOre: {
                    int cpx = px + constants::TILE_SIZE / 2;
                    int cpy = py + constants::TILE_SIZE / 2;
                    Color oreColor;
                    Color oreHighlight;
                    if (id == TileId::CopperOre) {
                        oreColor = {200, 120, 60, 255};
                        oreHighlight = {180, 100, 50, 255};
                    } else if (id == TileId::IronOre) {
                        oreColor = {200, 170, 120, 255};
                        oreHighlight = {180, 150, 100, 255};
                    } else {
                        oreColor = {255, 215, 0, 255};
                        oreHighlight = {220, 200, 40, 255};
                    }
                    DrawCircle(cpx, cpy, 3, oreColor);
                    DrawCircle(cpx - 2, cpy - 2, 2, oreHighlight);
                    break;
                }
                case TileId::Wood:
                    drawTileOverlay(px, py, Color{80, 50, 30, 255}, 2);
                    break;
                case TileId::Leaf:
                    drawTileOverlay(px, py, Color{30, 120, 30, 255}, 2);
                    break;
                case TileId::Granite:
                    drawTileOverlay(px, py, Color{100, 90, 110, 255}, 1);
                    break;
                case TileId::Marble:
                    drawTileOverlay(px, py, Color{220, 220, 230, 255}, 1);
                    break;
                case TileId::JungleGrass:
                    drawTileOverlay(px, py, Color{30, 120, 30, 255}, 4);
                    break;
                case TileId::Mud:
                    drawTileOverlay(px, py, Color{80, 60, 40, 255}, 1);
                    break;
                case TileId::Hellstone:
                    drawTileOverlay(px, py, Color{255, 100, 50, 100}, 2);
                    break;
                case TileId::Planks:
                    drawTileOverlay(px, py, Color{130, 90, 50, 255}, 1);
                    break;
                case TileId::Torch:
                    DrawCircle(px + constants::TILE_SIZE / 2, py + constants::TILE_SIZE / 2,
                               constants::TILE_SIZE * 2.5f,
                               Color{255, 200, 50, 30});
                    DrawCircle(px + constants::TILE_SIZE / 2, py + constants::TILE_SIZE / 2,
                               constants::TILE_SIZE * 1.5f,
                               Color{255, 220, 100, 50});
                    DrawCircle(px + constants::TILE_SIZE / 2, py + constants::TILE_SIZE / 2,
                               constants::TILE_SIZE * 0.8f,
                               Color{255, 230, 150, 80});
                    break;
                default:
                    break;
            }
        }
    }

    for (int tileX = startTileX; tileX <= endTileX; ++tileX) {
        for (int tileY = startTileY; tileY <= endTileY; ++tileY) {
            uint8_t water = world.getWater(tileX, tileY);
            uint8_t lava = world.getLava(tileX, tileY);
            if (water == 0 && lava == 0) continue;

            int px = static_cast<int>(math::tileToWorldX(tileX));
            int py = static_cast<int>(math::tileToWorldY(tileY));

            if (world.isSolid(tileX, tileY))
                continue;

            if (water > 0) {
                float fill = static_cast<float>(water) / static_cast<float>(MAX_LIQUID_LEVEL);
                int liquidH = std::max(1, static_cast<int>(constants::TILE_SIZE * fill));
                int liquidY = py + constants::TILE_SIZE - liquidH;
                unsigned char alpha = static_cast<unsigned char>(100 + 80 * fill);
                DrawRectangle(px, liquidY, constants::TILE_SIZE, liquidH,
                              Color{40, 120, 220, alpha});

                bool isTopSurface = tileY - 1 < 0 ||
                    (!world.isInBounds(tileX, tileY - 1)) ||
                    (world.isSolid(tileX, tileY - 1)) ||
                    world.getWater(tileX, tileY - 1) == 0;
                if (fill > 0.5f && isTopSurface) {
                    DrawRectangle(px, liquidY, constants::TILE_SIZE, 1,
                                  Color{80, 160, 255, alpha});
                }
            }

            if (lava > 0) {
                float fill = static_cast<float>(lava) / static_cast<float>(MAX_LIQUID_LEVEL);
                int liquidH = std::max(1, static_cast<int>(constants::TILE_SIZE * fill));
                int liquidY = py + constants::TILE_SIZE - liquidH;
                unsigned char alpha = static_cast<unsigned char>(180 + 75 * fill);
                DrawRectangle(px, liquidY, constants::TILE_SIZE, liquidH,
                              Color{255, 80, 0, alpha});

                bool isTopSurface = tileY - 1 < 0 ||
                    (!world.isInBounds(tileX, tileY - 1)) ||
                    (world.isSolid(tileX, tileY - 1)) ||
                    world.getLava(tileX, tileY - 1) == 0;
                if (fill > 0.5f && isTopSurface) {
                    DrawRectangle(px, liquidY, constants::TILE_SIZE, 1,
                                  Color{255, 180, 50, alpha});
                }
            }
        }
    }
}

bool isSolidLightingTile(const World& world, int x, int y) {
    if (!world.isInBounds(x, y))
        return false;
    return world.isSolid(x, y);
}

int findMainSurfaceY(const World& world, int tileX) {
    constexpr int CHECK_DEPTH = 48;
    constexpr int REQUIRED_SOLID = 30;
    constexpr int MAX_AIR_GAP = 8;

    for (int y = 0; y < constants::WORLD_HEIGHT; ++y) {
        if (!isSolidLightingTile(world, tileX, y))
            continue;

        int solidCount = 0;
        int currentAirGap = 0;
        int largestAirGap = 0;

        int endY = std::min(constants::WORLD_HEIGHT, y + CHECK_DEPTH);

        for (int cy = y; cy < endY; ++cy) {
            if (isSolidLightingTile(world, tileX, cy)) {
                solidCount++;
                currentAirGap = 0;
            } else {
                currentAirGap++;
                largestAirGap = std::max(largestAirGap, currentAirGap);
            }
        }

        if (solidCount >= REQUIRED_SOLID && largestAirGap <= MAX_AIR_GAP)
            return y;
    }

    return constants::WORLD_HEIGHT;
}

void RenderSystem::renderLightingOverlay(const World& world,
                                         const Camera2D& camera,
                                         Vector2 playerLightPos) {
    float viewLeft   = camera.target.x - camera.offset.x;
    float viewTop    = camera.target.y - camera.offset.y;
    float viewRight  = viewLeft + constants::SCREEN_WIDTH;
    float viewBottom = viewTop + constants::SCREEN_HEIGHT;

    int startTileX = std::max(0, math::worldToTileX(viewLeft) - 2);
    int endTileX   = std::min(constants::WORLD_WIDTH - 1, math::worldToTileX(viewRight) + 2);

    std::vector<LightSrc> lights;

    int startTileY = std::max(0, math::worldToTileY(viewTop) - 2);
    int endTileY   = std::min(constants::WORLD_HEIGHT - 1, math::worldToTileY(viewBottom) + 2);

    for (int tileX = startTileX; tileX <= endTileX; ++tileX) {
        for (int tileY = startTileY; tileY <= endTileY; ++tileY) {
            if (world.getTile(tileX, tileY) == TileId::Torch) {
                float cx = math::tileToWorldX(tileX) + constants::TILE_SIZE / 2.0f;
                float cy = math::tileToWorldY(tileY) + constants::TILE_SIZE / 2.0f;
                lights.push_back({cx, cy, TORCH_RADIUS, 1.0f});
            }
        }
    }

    if (playerLightPos.x >= 0.0f && playerLightPos.y >= 0.0f) {
        lights.push_back({
            playerLightPos.x,
            playerLightPos.y,
            PLAYER_LIGHT_RADIUS,
            0.65f
        });
    }

    int playerTileX = math::worldToTileX(playerLightPos.x);
    int playerTileY = math::worldToTileY(playerLightPos.y);
    int sy = findMainSurfaceY(world, playerTileX);
    int depth = playerTileY - sy;

    if (depth > 0) {
        float t = smooth01(static_cast<float>(depth) / DEPTH_FALLOFF);
        float darkness = (1.0f - SURFACE_LIGHT) + (SURFACE_LIGHT - UNDERGROUND_LIGHT) * t;
        darkness = clamp01(darkness);
        unsigned char alpha = static_cast<unsigned char>(darkness * MAX_DARKNESS);
        if (alpha > 3) {
            DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                          Color{0, 0, 0, alpha});
        }
    }

    BeginBlendMode(BLEND_ADDITIVE);
    for (const auto& src : lights) {
        DrawCircleGradient(static_cast<int>(src.x),
                           static_cast<int>(src.y),
                           src.radius * 0.45f,
                           Color{255, 190, 80, 45},
                           Color{255, 120, 20, 0});
    }
    EndBlendMode();
}
