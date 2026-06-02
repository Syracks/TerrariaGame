#include "MusicManager.hpp"
#include "world/Tile.hpp"

#include <raylib.h>

MusicManager& MusicManager::instance() {
    static MusicManager mgr;
    return mgr;
}

MusicManager::~MusicManager() {
    unloadAll();
}

void MusicManager::loadAll() {
    unloadAll();

    auto load = [](const char* path) {
        Music m = {};
        if (FileExists(path)) {
            m = LoadMusicStream(path);
        }
        return m;
    };

    m_forest = load("assets/sounds/forest.mp3");
    m_desert = load("assets/sounds/desert.mp3");
    m_ice = load("assets/sounds/ice.mp3");
    m_jungle = load("assets/sounds/jungle.mp3");
    m_ocean = load("assets/sounds/ocean.mp3");
    m_underground = load("assets/sounds/underground.mp3");
    m_night = load("assets/sounds/night.mp3");
    m_boss = load("assets/sounds/boss.mp3");

    m_loaded = true;
}

void MusicManager::unloadAll() {
    stopCurrent();
    auto unload = [](Music& m) {
        if (m.stream.buffer != nullptr) {
            UnloadMusicStream(m);
            m = {};
        }
    };
    unload(m_forest);
    unload(m_desert);
    unload(m_ice);
    unload(m_jungle);
    unload(m_ocean);
    unload(m_underground);
    unload(m_night);
    unload(m_boss);
    m_loaded = false;
}

void MusicManager::stopCurrent() {
    if (m_current.stream.buffer != nullptr) {
        StopMusicStream(m_current);
        m_current = {};
    }
}

void MusicManager::playTrack(Music music) {
    if (music.stream.buffer == nullptr) return;
    m_current = music;
    SetMusicVolume(m_current, 1.0f);
    PlayMusicStream(m_current);
}

void MusicManager::update(Biome biome, bool bossAlive, float dayTime) {
    if (!m_loaded) return;

    m_lastBossAlive = bossAlive;
    m_lastBiome = biome;

    Music target{};

    if (bossAlive) {
        target = m_boss;
    } else {
        float cyclePos = dayTime / 420.0f;
        float hour = cyclePos * 24.0f + 8.0f;
        bool isNight = hour < 8.0f || hour >= 20.0f;
        if (isNight) {
            target = m_night;
        } else {
            switch (biome) {
                case Biome::Desert:  target = m_desert; break;
                case Biome::Snow:    target = m_ice; break;
                case Biome::Jungle:  target = m_jungle; break;
                case Biome::Ocean:
                case Biome::Beach:   target = m_ocean; break;
                default:             target = m_forest; break;
            }
        }
    }

    if (target.stream.buffer != m_current.stream.buffer) {
        stopCurrent();
        if (target.stream.buffer != nullptr) {
            playTrack(target);
        }
    }
}

void MusicManager::stop() {
    stopCurrent();
}

void MusicManager::tick() {
    if (!m_loaded) return;
    if (m_current.stream.buffer != nullptr) {
        UpdateMusicStream(m_current);
    }
}
