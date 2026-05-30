#include "entities/Player.hpp"
#include "core/Constants.hpp"
#include "core/TextureManager.hpp"
#include "core/SoundManager.hpp"
#include "items/Tool.hpp"

#include <cmath>

namespace {
    constexpr float PLAYER_WIDTH = 16.0f;
    constexpr float PLAYER_HEIGHT = 32.0f;
    constexpr float FRAME_TIME = 0.12f;

    constexpr int WALK_Y = 34;
    constexpr int WALK_H = 188;
    constexpr int WALK_X[] = {69, 325, 581, 838, 1093};
    constexpr int WALK_W[] = {104, 105, 105, 103, 105};

    constexpr int JUMP_Y = 35;
    constexpr int JUMP_H = 200;
    constexpr int JUMP_X[] = {67, 330, 580, 836, 1092};
    constexpr int JUMP_W[] = {134, 126, 138, 157, 139};

    constexpr float SWING_ARC = 120.0f;
    constexpr float SWING_START_ANGLE = -60.0f;
    constexpr float HITBOX_ACTIVE_START = 0.15f;
    constexpr float HITBOX_ACTIVE_END = 0.65f;
    constexpr float TOOL_SIZE = 24.0f;
}

Player::Player()
    : m_reach(constants::PLAYER_REACH * constants::TILE_SIZE) {
    m_width = PLAYER_WIDTH;
    m_height = PLAYER_HEIGHT;
}

static Texture2D loadTexture(const char* path) {
    Texture2D tex{};
    Image img = LoadImage(path);
    if (img.data != nullptr) {
        tex = LoadTextureFromImage(img);
        UnloadImage(img);
    }
    return tex;
}

void Player::buildWalkFrames() {
    for (int row = 0; row < ANIM_ROWS; ++row) {
        int y = WALK_Y + row * (WALK_H + 68);
        for (int col = 0; col < ANIM_COLS; ++col) {
            m_walkFrames[row * ANIM_COLS + col] = {WALK_X[col], y, WALK_W[col], WALK_H};
        }
    }
}

void Player::buildJumpFrames() {
    const int jumpRows[] = {35, 289, 514, 804, 1062};
    for (int row = 0; row < ANIM_ROWS; ++row) {
        int y = jumpRows[row];
        for (int col = 0; col < ANIM_COLS; ++col) {
            m_jumpFrames[row * ANIM_COLS + col] = {JUMP_X[col], y, JUMP_W[col], JUMP_H};
        }
    }
}

void Player::load() {
    unload();
    m_idleTex = loadTexture("assets/textures/player/player.png");
    m_walkTex = loadTexture("assets/textures/player/player_walk.png");
    m_jumpTex = loadTexture("assets/textures/player/player_jump.png");
    buildWalkFrames();
    buildJumpFrames();
}

void Player::unload() {
    if (m_idleTex.id > 0) { UnloadTexture(m_idleTex); m_idleTex = {}; }
    if (m_walkTex.id > 0) { UnloadTexture(m_walkTex); m_walkTex = {}; }
    if (m_jumpTex.id > 0) { UnloadTexture(m_jumpTex); m_jumpTex = {}; }
}

Player::~Player() {
    unload();
}

void Player::advanceAnimation(float dt, int frameCount) {
    m_frameTimer += dt;
    if (m_frameTimer >= FRAME_TIME) {
        m_frameTimer = 0.0f;
        m_currentFrame = (m_currentFrame + 1) % frameCount;
    }
}

void Player::startSwing() {
    startSwing(m_swingDuration);
}

void Player::startSwing(float customDuration) {
    if (m_isSwinging) return;
    auto* sel = m_inventory.getSelectedSlot();
    if (!sel || sel->count <= 0) return;
    if (!isTool(sel->tileId)) return;
    m_isSwinging = true;
    m_swingTimer = 0.0f;
    m_swingDuration = customDuration;
    m_swingJustCompleted = false;
}

void Player::clearInventory() {
    m_inventory = Inventory{};
}

bool Player::wasSwingJustCompleted() {
    bool ret = m_swingJustCompleted;
    m_swingJustCompleted = false;
    return ret;
}

void Player::setMiningTarget(int tx, int ty) {
    m_miningTargetX = tx;
    m_miningTargetY = ty;
}

void Player::clearMiningTarget() {
    m_miningTargetX = -1;
    m_miningTargetY = -1;
}

float Player::getSwingProgress() const {
    if (!m_isSwinging) return 0.0f;
    return m_swingTimer / m_swingDuration;
}

Rectangle Player::getSwingHitbox() const {
    float t = getSwingProgress();
    if (t < HITBOX_ACTIVE_START || t > HITBOX_ACTIVE_END) return {};

    float centerX = m_position.x + m_width / 2;
    float centerY = m_position.y + m_height / 2;

    float angleDeg = SWING_START_ANGLE + t * SWING_ARC;
    float angleRad = angleDeg * DEG2RAD;

    float mult = m_facingLeft ? -1.0f : 1.0f;
    float dist = constants::TILE_SIZE * 1.5f;
    float hx = centerX + std::cos(angleRad) * dist * mult;
    float hy = centerY + std::sin(angleRad) * dist;

    float hs = constants::TILE_SIZE * 0.8f;
    return {hx - hs / 2, hy - hs / 2, hs, hs};
}

void Player::update(float dt) {
    applyHorizontalMovement();
    applyGravity(dt);

    if (!m_onGround) {
        if (m_animState != AnimState::Jump) {
            m_animState = AnimState::Jump;
            m_currentFrame = 0;
            m_frameTimer = 0.0f;
        }
        advanceAnimation(dt, 5);
    } else if (std::abs(m_velocity.x) > 1.0f) {
        if (m_animState != AnimState::Walk) {
            m_animState = AnimState::Walk;
            m_currentFrame = 0;
            m_frameTimer = 0.0f;
        }
        advanceAnimation(dt, 5);
    } else {
        m_animState = AnimState::Idle;
        m_currentFrame = 0;
        m_frameTimer = 0.0f;
    }

    if (m_velocity.x < -0.1f) m_facingLeft = true;
    else if (m_velocity.x > 0.1f) m_facingLeft = false;

    if (m_hurtTimer > 0.0f) {
        m_hurtTimer -= dt;
        m_regenTimer = 0.0f;
    } else if (m_health < m_maxHealth) {
        m_regenTimer += dt;
        if (m_regenTimer >= 2.0f) {
            m_health++;
            m_regenTimer = 0.0f;
        }
    }

    m_swingJustCompleted = false;
    if (m_isSwinging) {
        m_swingTimer += dt;
        if (m_swingTimer >= m_swingDuration) {
            m_swingJustCompleted = true;
            m_isSwinging = false;
            m_swingTimer = 0.0f;
        }
    }
}

void Player::render() const {
    Texture2D* tex = nullptr;
    Rectangle src{};

    switch (m_animState) {
        case AnimState::Walk:
            if (m_walkTex.id > 0) {
                tex = const_cast<Texture2D*>(&m_walkTex);
                const auto& f = m_walkFrames[m_currentFrame];
                src = {static_cast<float>(f.x), static_cast<float>(f.y),
                       static_cast<float>(f.w), static_cast<float>(f.h)};
            }
            break;
        case AnimState::Jump:
            if (m_jumpTex.id > 0) {
                tex = const_cast<Texture2D*>(&m_jumpTex);
                const auto& f = m_jumpFrames[m_currentFrame];
                src = {static_cast<float>(f.x), static_cast<float>(f.y),
                       static_cast<float>(f.w), static_cast<float>(f.h)};
            }
            break;
        case AnimState::Idle:
            if (m_idleTex.id > 0) {
                tex = const_cast<Texture2D*>(&m_idleTex);
                src = {0, 0, static_cast<float>(m_idleTex.width),
                       static_cast<float>(m_idleTex.height)};
            }
            break;
    }

    if (tex && tex->id > 0) {
        Rectangle dst = {m_position.x, m_position.y, m_width, m_height};
        if (m_facingLeft) {
            src.width = -src.width;
        }
        DrawTexturePro(*tex, src, dst, {0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(getBounds(), BLUE);
    }

    if (m_isSwinging) {
        auto* sel = m_inventory.getSelectedSlot();
        if (sel && sel->count > 0) {
            TileId toolId = sel->tileId;
            const Texture2D& toolTex = TextureManager::instance().getTexture(toolId);
            if (toolTex.id > 0) {
                float centerX = m_position.x + m_width / 2;
                float centerY = m_position.y + m_height / 2;
                float t = getSwingProgress();
                float angleDeg = SWING_START_ANGLE + t * SWING_ARC;

                float armLen = constants::TILE_SIZE * 0.9f;
                float angleRad = angleDeg * DEG2RAD;
                int side = m_facingLeft ? -1 : 1;
                float xOff = m_facingLeft ? 0.0f : constants::TILE_SIZE * 0.85f;
                float handX = centerX + std::cos(angleRad) * armLen * side + xOff;
                float handY = centerY + std::sin(angleRad) * armLen;

                Rectangle tSrc = {0, 0, static_cast<float>(toolTex.width),
                                  static_cast<float>(toolTex.height)};
                if (m_facingLeft) tSrc.width = -tSrc.width;
                Rectangle tDst = {handX - TOOL_SIZE / 2, handY - TOOL_SIZE / 2,
                                  TOOL_SIZE, TOOL_SIZE};
                float rot = m_facingLeft ? -angleDeg : angleDeg;
                DrawTexturePro(toolTex, tSrc, tDst, {TOOL_SIZE / 2, TOOL_SIZE / 2}, rot, WHITE);
            }
        }
    }
}

void Player::takeDamage(int amount) {
    if (m_hurtTimer > 0.0f) return;
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    m_hurtTimer = 0.5f;
}

void Player::heal(int amount) {
    m_health += amount;
    if (m_health > m_maxHealth) m_health = m_maxHealth;
}

void Player::moveLeft() {
    m_moveDirection = -1;
}

void Player::moveRight() {
    m_moveDirection = 1;
}

void Player::stopMoving() {
    m_moveDirection = 0;
}

void Player::jump() {
    if (!m_onGround) return;
    m_velocity.y = constants::JUMP_SPEED;
    m_onGround = false;
    SoundManager::instance().play(SoundManager::Jump);
}

Inventory& Player::getInventory() noexcept {
    return m_inventory;
}

const Inventory& Player::getInventory() const noexcept {
    return m_inventory;
}

float Player::getReach() const noexcept {
    return m_reach;
}

void Player::applyHorizontalMovement() {
    m_velocity.x = static_cast<float>(m_moveDirection) * constants::PLAYER_SPEED;
}

void Player::applyGravity(float dt) {
    m_velocity.y += constants::GRAVITY * dt;
}
