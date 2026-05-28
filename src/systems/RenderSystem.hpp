#pragma once

#include <raylib.h>

class World;

class RenderSystem {
public:
    static void renderWorld(const World& world, const Camera2D& camera,
                            float nightFactor);

    static void renderLightingOverlay(const World& world,
                                      const Camera2D& camera,
                                      float nightFactor,
                                      Vector2 playerLightPos);
};
