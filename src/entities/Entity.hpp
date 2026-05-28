#pragma once

#include <raylib.h>

class Entity {
public:
    Entity() = default;
    virtual ~Entity() = default;

    virtual void update(float dt) = 0;
    virtual void render() const = 0;

    Vector2 getPosition() const { return m_position; }
    void setPosition(Vector2 pos) { m_position = pos; }
    Vector2 getVelocity() const { return m_velocity; }
    void setVelocity(Vector2 vel) { m_velocity = vel; }
    Rectangle getBounds() const { return {m_position.x, m_position.y, m_width, m_height}; }
    bool isOnGround() const { return m_onGround; }
    void setOnGround(bool v) { m_onGround = v; }

protected:
    Vector2 m_position{};
    Vector2 m_velocity{};
    float m_width = 20.0f;
    float m_height = 30.0f;
    bool m_onGround = false;
};
