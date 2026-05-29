#pragma once

#include <raylib.h>

class SettingsMenu {
public:
    enum class Action {
        Back,
        None
    };

    SettingsMenu();
    Action update();
    void render() const;

private:
    int m_selectedOption = 0;
    float m_volume = 1.0f;
};
