#pragma once

#include <raylib.h>
#include <string>

enum class Biome;

class MusicManager {
public:
    static MusicManager& instance();

    void loadAll();
    void unloadAll();
    void update(Biome biome, bool bossAlive, float dayTime);
    void stop();
    void tick();

private:
    MusicManager() = default;
    ~MusicManager();
    MusicManager(const MusicManager&) = delete;
    MusicManager& operator=(const MusicManager&) = delete;

    void playTrack(Music music);
    void stopCurrent();

    Music m_forest{};
    Music m_desert{};
    Music m_ice{};
    Music m_jungle{};
    Music m_ocean{};
    Music m_underground{};
    Music m_night{};
    Music m_boss{};

    Music m_current{};
    Biome m_lastBiome = static_cast<Biome>(-1);
    bool m_lastBossAlive = false;
    bool m_loaded = false;
};
