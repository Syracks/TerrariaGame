#pragma once

#include <raylib.h>
#include <array>

class SoundManager {
public:
    enum Effect {
        Jump,
        PickaxeMine,
        AxeMine,
        SwordSwing,
        SwordHit,
        MobDeath,
        PlayerDeath,
        BlockPlace,
        TorchPlace,
        Craft,
        ItemPickup,
        Count
    };

    static SoundManager& instance();

    bool loadAll();
    void unloadAll();
    void play(Effect effect);
    void setMasterVolume(float volume);

private:
    SoundManager() = default;
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    Wave generateJump();
    Wave generatePickaxeMine();
    Wave generateAxeMine();
    Wave generateSwordSwing();
    Wave generateSwordHit();
    Wave generateMobDeath();
    Wave generatePlayerDeath();
    Wave generateBlockPlace();
    Wave generateTorchPlace();
    Wave generateCraft();
    Wave generateItemPickup();

    std::array<Sound, Count> m_sounds{};
    bool m_loaded = false;
};
