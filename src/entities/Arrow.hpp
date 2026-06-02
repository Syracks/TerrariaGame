#pragma once

#include <raylib.h>

struct Arrow {
    Vector2 position{};
    Vector2 velocity{};
    float lifetime = 2.0f;
    bool active = true;
    int facing = 1;
};
