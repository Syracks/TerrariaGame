#include "app/PlayerController.hpp"
#include "core/Input.hpp"

void PlayerController::update(Player& player) {
    if (input::isLeftPressed() && !input::isRightPressed()) {
        player.moveLeft();
    } else if (input::isRightPressed() && !input::isLeftPressed()) {
        player.moveRight();
    } else {
        player.stopMoving();
    }

    if (input::isJumpHeld()) {
        player.jump();
    }
}
