#pragma once

#include "entities/Player.hpp"

class PlayerController {
public:
    PlayerController() = default;

    void update(Player& player);
};
