#pragma once

#include <raylib.h>

struct TempHitbox {
    Rectangle bounds{};
    float lifetime = 0.5f;
    int damage = 10;
    bool active = true;
    bool targetsPlayer = false;
};
