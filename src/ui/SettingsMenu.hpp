#pragma once

#include <raylib.h>
#include "core/Constants.hpp"

class SettingsMenu {
public:
    enum class Action {
        Back,
        None
    };

    SettingsMenu();

    Action update();
    void render() const;

    int getResolutionIndex() const { return m_resolutionIndex; }
    void setResolutionIndex(int idx) { m_resolutionIndex = idx; }
    bool isFullscreen() const { return m_fullscreen; }
    void setFullscreen(bool v) { m_fullscreen = v; }
    bool getShowMinimap() const { return m_showMinimap; }
    void setShowMinimap(bool v) { m_showMinimap = v; }
    float getVolume() const { return m_volume; }
    void setVolume(float v) { m_volume = v; }

    void saveToFile() const;
    void loadFromFile();

private:
    int m_selectedOption = 0;
    float m_volume = 1.0f;
    int m_resolutionIndex = 0;
    bool m_fullscreen = false;
    bool m_showMinimap = true;
};
