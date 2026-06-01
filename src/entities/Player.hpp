#pragma once

#include "Entity.hpp"
#include "items/Inventory.hpp"
#include <raylib.h>
#include <array>

class Player final : public Entity {
public:
    Player();
    ~Player();

    void load();
    void unload();
    void update(float dt) override;
    void render() const override;

    void moveLeft();
    void moveRight();
    void stopMoving();
    void jump();
    void startSwing();
    void startSwing(float customDuration);

    bool isSwinging() const { return m_isSwinging; }
    bool wasSwingJustCompleted();
    float getSwingProgress() const;
    Rectangle getSwingHitbox() const;

    void setMiningTarget(int tx, int ty);
    int getMiningTargetX() const { return m_miningTargetX; }
    int getMiningTargetY() const { return m_miningTargetY; }
    void clearMiningTarget();

    int getHealth() const { return m_health; }
    int getMaxHealth() const { return m_maxHealth; }
    void takeDamage(int amount);
    void heal(int amount);

    Inventory& getInventory() noexcept;
    const Inventory& getInventory() const noexcept;

    float getReach() const noexcept;
    void clearInventory();

private:
    void applyHorizontalMovement();
    void applyGravity(float dt);
    void advanceAnimation(float dt, int frameCount);

    void buildWalkFrames();
    void buildJumpFrames();

    static constexpr int ANIM_COLS = 5;
    static constexpr int ANIM_ROWS = 5;

    struct FrameInfo {
        int x, y, w, h;
    };

private:
    Inventory m_inventory;

    mutable Texture2D m_idleTex{};
    mutable Texture2D m_walkTex{};
    mutable Texture2D m_jumpTex{};

    std::array<FrameInfo, 25> m_walkFrames;
    std::array<FrameInfo, 25> m_jumpFrames;

    enum class AnimState { Idle, Walk, Jump };
    AnimState m_animState = AnimState::Idle;
    int m_currentFrame = 0;
    float m_frameTimer = 0.0f;
    bool m_facingLeft = false;

    float m_reach = 0.0f;
    int m_moveDirection = 0;

    bool m_isSwinging = false;
    float m_swingTimer = 0.0f;
    float m_swingDuration = 0.35f;
    bool m_swingJustCompleted = false;

    int m_miningTargetX = -1;
    int m_miningTargetY = -1;

    int m_health = 100;
    int m_maxHealth = 100;
    float m_hurtTimer = 0.0f;
    float m_regenTimer = 0.0f;
};
