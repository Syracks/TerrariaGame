#include "CameraController.hpp"
#include "entities/Player.hpp"
#include "core/Constants.hpp"

CameraController::CameraController() {
    m_camera.target = {0, 0};
    m_camera.offset = {
        constants::SCREEN_WIDTH / 2.0f,
        constants::SCREEN_HEIGHT / 2.0f
    };
    m_camera.rotation = 0.0f;
    m_camera.zoom = 2.0f;
}

void CameraController::update(const Player& player) {
    Vector2 playerCenter = {
        player.getPosition().x + player.getBounds().width / 2.0f,
        player.getPosition().y + player.getBounds().height / 2.0f
    };
    m_camera.target = playerCenter;
}
