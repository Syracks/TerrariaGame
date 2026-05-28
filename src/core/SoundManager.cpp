#include "SoundManager.hpp"

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

namespace {
    constexpr int SAMPLE_RATE = 22050;

    struct SampleBuffer {
        std::vector<int16_t> data;
        int sampleRate;

        void write(float value) {
            value = std::max(-1.0f, std::min(1.0f, value));
            data.push_back(static_cast<int16_t>(value * 32760.0f));
        }

        void fillSine(float freq, float duration, float volume = 1.0f) {
            int n = static_cast<int>(sampleRate * duration);
            for (int i = 0; i < n; ++i) {
                float t = static_cast<float>(i) / sampleRate;
                float env = 1.0f;
                float attack = 0.003f;
                float release = 0.005f;
                if (t < attack) env = t / attack;
                if (t > duration - release) env = (duration - t) / release;
                write(env * volume * sinf(2.0f * M_PI * freq * t));
            }
        }

        void fillSineSweep(float freqStart, float freqEnd, float duration, float volume = 1.0f) {
            int n = static_cast<int>(sampleRate * duration);
            for (int i = 0; i < n; ++i) {
                float t = static_cast<float>(i) / sampleRate;
                float progress = t / duration;
                float freq = freqStart + (freqEnd - freqStart) * progress;
                float env = 1.0f;
                float attack = 0.003f;
                float release = 0.01f;
                if (t < attack) env = t / attack;
                if (t > duration - release) env = (duration - t) / release;
                write(env * volume * sinf(2.0f * M_PI * freq * t));
            }
        }

        void fillNoise(float duration, float volume = 1.0f) {
            int n = static_cast<int>(sampleRate * duration);
            for (int i = 0; i < n; ++i) {
                float t = static_cast<float>(i) / sampleRate;
                float env = 1.0f;
                float release = 0.003f;
                if (t > duration - release) env = (duration - t) / release;
                float noise = (static_cast<float>(std::rand()) / RAND_MAX) * 2.0f - 1.0f;
                write(env * volume * noise);
            }
        }

        void addSineSweep(float freqStart, float freqEnd, float duration, float volume = 1.0f) {
            int n = static_cast<int>(sampleRate * duration);
            for (int i = 0; i < n && i < static_cast<int>(data.size()); ++i) {
                float t = static_cast<float>(i) / sampleRate;
                float progress = t / duration;
                float freq = freqStart + (freqEnd - freqStart) * progress;
                float env = 1.0f;
                float attack = 0.003f;
                if (t < attack) env = t / attack;
                float value = env * volume * sinf(2.0f * M_PI * freq * t);
                float existing = static_cast<float>(data[i]) / 32760.0f;
                float mixed = std::max(-1.0f, std::min(1.0f, existing + value));
                data[i] = static_cast<int16_t>(mixed * 32760.0f);
            }
        }

        Wave toWave() {
            Wave w{};
            w.frameCount = data.size();
            w.sampleRate = sampleRate;
            w.sampleSize = 16;
            w.channels = 1;
            w.data = malloc(data.size() * sizeof(int16_t));
            if (w.data) {
                memcpy(w.data, data.data(), data.size() * sizeof(int16_t));
            }
            return w;
        }
    };
}

SoundManager& SoundManager::instance() {
    static SoundManager mgr;
    return mgr;
}

SoundManager::~SoundManager() {
    unloadAll();
}

bool SoundManager::loadAll() {
    unloadAll();

    auto makeSound = [](Wave wave) {
        Sound s = LoadSoundFromWave(wave);
        UnloadWave(wave);
        return s;
    };

    m_sounds[Jump]          = makeSound(generateJump());
    m_sounds[PickaxeMine]   = makeSound(generatePickaxeMine());
    m_sounds[AxeMine]       = makeSound(generateAxeMine());
    m_sounds[SwordSwing]    = makeSound(generateSwordSwing());
    m_sounds[SwordHit]      = makeSound(generateSwordHit());
    m_sounds[MobDeath]      = makeSound(generateMobDeath());
    m_sounds[PlayerDeath]   = makeSound(generatePlayerDeath());
    m_sounds[BlockPlace]    = makeSound(generateBlockPlace());
    m_sounds[TorchPlace]    = makeSound(generateTorchPlace());
    m_sounds[Craft]         = makeSound(generateCraft());
    m_sounds[ItemPickup]    = makeSound(generateItemPickup());

    m_loaded = true;
    return true;
}

void SoundManager::unloadAll() {
    if (!m_loaded) return;
    for (auto& s : m_sounds) {
        if (s.stream.buffer != nullptr) {
            UnloadSound(s);
            s = {};
        }
    }
    m_loaded = false;
}

void SoundManager::play(Effect effect) {
    if (!m_loaded || effect < 0 || effect >= Count) return;
    PlaySound(m_sounds[effect]);
}

void SoundManager::setMasterVolume(float volume) {
    SetMasterVolume(volume);
}



Wave SoundManager::generateJump() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSineSweep(300.0f, 700.0f, 0.1f, 0.5f);
    return buf.toWave();
}

Wave SoundManager::generatePickaxeMine() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillNoise(0.04f, 0.6f);
    buf.addSineSweep(400.0f, 200.0f, 0.06f, 0.4f);
    return buf.toWave();
}

Wave SoundManager::generateAxeMine() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSine(150.0f, 0.08f, 0.7f);
    buf.addSineSweep(300.0f, 100.0f, 0.06f, 0.3f);
    return buf.toWave();
}

Wave SoundManager::generateSwordSwing() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillNoise(0.12f, 0.4f);
    buf.addSineSweep(200.0f, 600.0f, 0.1f, 0.2f);
    return buf.toWave();
}

Wave SoundManager::generateSwordHit() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillNoise(0.03f, 0.8f);
    buf.addSineSweep(800.0f, 400.0f, 0.07f, 0.6f);
    return buf.toWave();
}

Wave SoundManager::generateMobDeath() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSineSweep(250.0f, 40.0f, 0.3f, 0.6f);
    buf.addSineSweep(300.0f, 30.0f, 0.25f, 0.3f);
    return buf.toWave();
}

Wave SoundManager::generatePlayerDeath() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSineSweep(400.0f, 20.0f, 0.6f, 0.7f);
    buf.addSineSweep(500.0f, 15.0f, 0.5f, 0.4f);
    return buf.toWave();
}

Wave SoundManager::generateBlockPlace() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSine(120.0f, 0.07f, 0.6f);
    buf.addSineSweep(50.0f, 20.0f, 0.04f, 0.3f);
    return buf.toWave();
}

Wave SoundManager::generateTorchPlace() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillNoise(0.03f, 0.4f);
    buf.fillSine(600.0f, 0.02f, 0.2f);
    return buf.toWave();
}

Wave SoundManager::generateCraft() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSineSweep(440.0f, 880.0f, 0.15f, 0.5f);
    buf.addSineSweep(660.0f, 1200.0f, 0.1f, 0.3f);
    return buf.toWave();
}

Wave SoundManager::generateItemPickup() {
    SampleBuffer buf{{}, SAMPLE_RATE};
    buf.fillSineSweep(600.0f, 1200.0f, 0.07f, 0.5f);
    return buf.toWave();
}
