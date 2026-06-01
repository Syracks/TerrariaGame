#include "entities/Mob.hpp"
#include "core/Constants.hpp"

#include <cmath>
#include <random>

namespace {
    constexpr float SLIME_W = 16.0f;
    constexpr float SLIME_H = 16.0f;
    constexpr int SLIME_HP = 50;
    constexpr int SLIME_DMG = 10;

    constexpr float BLUE_SLIME_W = 24.0f;
    constexpr float BLUE_SLIME_H = 24.0f;
    constexpr int BLUE_SLIME_HP = 75;
    constexpr int BLUE_SLIME_DMG = 20;

    constexpr float ZOMBIE_W = 16.0f;
    constexpr float ZOMBIE_H = 28.0f;
    constexpr int ZOMBIE_HP = 75;
    constexpr int ZOMBIE_DMG = 15;
    constexpr float ZOMBIE_SPEED = 50.0f;

    constexpr float IDLE_HOP_INTERVAL = 0.8f;
    constexpr float IDLE_HOP_VY = -300.0f;
    constexpr float IDLE_HOP_VX = 60.0f;

    constexpr float CHASE_HOP_VY = -250.0f;
    constexpr float CHASE_HOP_VX = 100.0f;
    constexpr float CHASE_DURATION = 8.0f;
    constexpr float CHASE_STOP_DIST = 20.0f;

    constexpr float HIT_COOLDOWN = 0.5f;
    constexpr float FRAME_TIME = 0.2f;

    constexpr float KB_FORCE_SLIME = 300.0f;
    constexpr float KB_FORCE_BLUE_SLIME = 150.0f;
    constexpr float KB_FORCE_ZOMBIE = 200.0f;

    constexpr float ZOMBIE_PATROL_SPEED = 30.0f;
    constexpr float ZOMBIE_CHASE_SPEED = 60.0f;
    constexpr float ZOMBIE_TURN_INTERVAL = 3.0f;
    constexpr float ZOMBIE_JUMP_VY = -350.0f;
}

Mob::Mob(MobType type, Vector2 position)
    : m_type(type) {
    m_position = position;
    switch (m_type) {
        case MobType::BlueSlime:
            m_width = BLUE_SLIME_W;
            m_height = BLUE_SLIME_H;
            m_health = BLUE_SLIME_HP;
            m_maxHealth = BLUE_SLIME_HP;
            break;
        case MobType::Zombie:
            m_width = ZOMBIE_W;
            m_height = ZOMBIE_H;
            m_health = ZOMBIE_HP;
            m_maxHealth = ZOMBIE_HP;
            break;
        default:
            m_width = SLIME_W;
            m_height = SLIME_H;
            m_health = SLIME_HP;
            m_maxHealth = SLIME_HP;
            break;
    }
}

Mob::~Mob() {
    unload();
}

void Mob::load() {
    unload();
    const char* path;
    switch (m_type) {
        case MobType::Zombie:
            path = "assets/textures/enemies/zombie.png";
            break;
        default:
            path = "assets/textures/enemies/slime.png";
            break;
    }
    Image img = LoadImage(path);
    if (img.data != nullptr) {
        if (m_type == MobType::BlueSlime) {
            ImageColorTint(&img, (Color){50, 130, 255, 255});
        }
        m_tex = LoadTextureFromImage(img);
        ImageFlipHorizontal(&img);
        m_texFlipped = LoadTextureFromImage(img);
        UnloadImage(img);
    }
}

void Mob::unload() {
    if (m_tex.id > 0) { UnloadTexture(m_tex); m_tex = {}; }
    if (m_texFlipped.id > 0) { UnloadTexture(m_texFlipped); m_texFlipped = {}; }
}

void Mob::takeDamage(int amount) {
    if (m_hitTimer > 0.0f) return;
    m_hitTimer = HIT_COOLDOWN;
    m_health -= amount;
    if (m_health < 0) m_health = 0;
    m_state = MobState::Chase;
    m_chaseDuration = CHASE_DURATION;
}

void Mob::update(float dt) {
    if (m_hitTimer > 0.0f) m_hitTimer -= dt;

    if (m_state == MobState::Chase) {
        m_chaseDuration -= dt;
        if (m_chaseDuration <= 0.0f) {
            m_state = MobState::Idle;
        }
        if (m_type == MobType::Zombie)
            zombieChaseAI(dt);
        else
            chaseAI(dt);
    } else {
        if (m_type == MobType::Zombie)
            zombieIdleAI(dt);
        else
            idleAI(dt);
    }

    m_frameTimer += dt;
    if (m_frameTimer >= FRAME_TIME) {
        m_frameTimer = 0.0f;
        m_frameIndex = (m_frameIndex + 1) % 3;
    }
}

void Mob::idleAI(float dt) {
    m_hopCooldown -= dt;
    if (m_hopCooldown <= 0.0f && m_onGround) {
        {
        static std::mt19937 rng(std::random_device{}());
        std::uniform_real_distribution<float> dist(0.0f, 1.0f);
        m_hopCooldown = IDLE_HOP_INTERVAL + dist(rng);
    }
        m_velocity.x = static_cast<float>(m_facing) * IDLE_HOP_VX;
        m_velocity.y = IDLE_HOP_VY;
    } else if (m_onGround) {
        m_velocity.x = 0.0f;
    }
}

void Mob::chaseAI(float dt) {
    m_chaseHopCooldown -= dt;
    float dx = m_playerPos.x - m_position.x;
    m_facing = (dx > 0) ? 1 : -1;

    if (m_onGround && m_chaseHopCooldown <= 0.0f) {
        m_velocity.x = static_cast<float>(m_facing) * CHASE_HOP_VX;
        m_velocity.y = CHASE_HOP_VY;
        m_chaseHopCooldown = 0.5f;
    } else if (m_onGround) {
        m_velocity.x = 0.0f;
    }
}

void Mob::zombieIdleAI(float dt) {
    m_hopCooldown -= dt;
    if (m_hopCooldown <= 0.0f) {
        m_hopCooldown = ZOMBIE_TURN_INTERVAL;
        m_facing = -m_facing;
    }
    if (m_onGround) {
        m_velocity.x = static_cast<float>(m_facing) * ZOMBIE_PATROL_SPEED;
    }
}

void Mob::zombieChaseAI(float dt) {
    float dx = m_playerPos.x - m_position.x;
    float dy = m_playerPos.y - m_position.y;
    m_facing = (dx > 0) ? 1 : -1;

    if (m_onGround) {
        m_velocity.x = static_cast<float>(m_facing) * ZOMBIE_CHASE_SPEED;
        if (dy < -constants::TILE_SIZE * 2 && std::abs(dx) < constants::TILE_SIZE * 4) {
            m_velocity.y = ZOMBIE_JUMP_VY;
        }
    }
}

void Mob::render() const {
    Texture2D* tex = m_facing == 1 ? &m_tex : &m_texFlipped;
    if (tex && tex->id > 0) {
        Rectangle src = {0, 0, static_cast<float>(m_tex.width),
                         static_cast<float>(m_tex.height)};
        Rectangle dst = {m_position.x, m_position.y, m_width, m_height};
        DrawTexturePro(*tex, src, dst, {0, 0}, 0.0f, WHITE);
    } else {
        DrawRectangleRec(getBounds(), {100, 200, 80, 255});
    }

    if (m_health < m_maxHealth) {
        int barW = static_cast<int>(m_width);
        int barH = 3;
        float barX = m_position.x;
        float barY = m_position.y - 5;
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY), barW, barH, Color{60, 60, 60, 200});
        int fillW = barW * m_health / m_maxHealth;
        DrawRectangle(static_cast<int>(barX), static_cast<int>(barY), fillW, barH, Color{220, 50, 50, 255});
    }
}

int Mob::getContactDamage() const {
    switch (m_type) {
        case MobType::BlueSlime: return BLUE_SLIME_DMG;
        case MobType::Zombie: return ZOMBIE_DMG;
        default: return SLIME_DMG;
    }
}

void Mob::knockback(Vector2 dir) {
    float force;
    switch (m_type) {
        case MobType::BlueSlime: force = KB_FORCE_BLUE_SLIME; break;
        case MobType::Zombie: force = KB_FORCE_ZOMBIE; break;
        default: force = KB_FORCE_SLIME; break;
    }
    m_velocity.x = dir.x * force;
    m_velocity.y = dir.y * force * 0.4f;
}
