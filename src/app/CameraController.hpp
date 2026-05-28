#pragma once

#include <raylib.h>

class Player;

class CameraController {
public:
    CameraController();
    void update(const Player& player);
    const Camera2D& getCamera() const { return m_camera; }

private:
    Camera2D m_camera;
};
