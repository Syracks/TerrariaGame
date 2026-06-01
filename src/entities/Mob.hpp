#pragma once

#include "Entity.hpp"
#include <raylib.h>

enum class MobType { Slime, BlueSlime, Zombie };
enum class MobState { Idle, Chase };

class Mob final : public Entity {
public:
    Mob(MobType type, Vector2 position);
    ~Mob();

    void load();
    void unload();
    void update(float dt) override;
    void render() const override;

    MobType getType() const { return m_type; }
    MobState getState() const { return m_state; }
    void setPlayerPos(Vector2 pos) { m_playerPos = pos; }
    void setFacing(int dir) { m_facing = dir; }
    bool isHitCooldown() const { return m_hitTimer > 0.0f; }
    void takeDamage(int amount);
    void knockback(Vector2 dir);
    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    int getContactDamage() const;

private:
    void applyPhysics(float dt);
    void idleAI(float dt);
    void chaseAI(float dt);
    void zombieIdleAI(float dt);
    void zombieChaseAI(float dt);

    MobType m_type;
    MobState m_state = MobState::Idle;

    mutable Texture2D m_tex{};
    mutable Texture2D m_texFlipped{};

    Vector2 m_playerPos{};
    float m_speed = 0.0f;
    int m_facing = 1;

    float m_hopTimer = 0.0f;
    float m_hopCooldown = 0.0f;
    bool m_isHopping = false;

    int m_health = 50;
    int m_maxHealth = 50;
    float m_chaseHopCooldown = 0.0f;
    float m_hitTimer = 0.0f;
    float m_chaseDuration = 0.0f;

    int m_frameIndex = 0;
    float m_frameTimer = 0.0f;
};
