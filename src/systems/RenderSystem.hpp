#pragma once

#include <raylib.h>

class World;

class RenderSystem {
public:
    static void renderWorld(const World& world, const Camera2D& camera);

    static void renderLightingOverlay(const World& world,
                                      const Camera2D& camera,
                                      Vector2 playerLightPos);
};
