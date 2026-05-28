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
    static constexpr int OPTION_COUNT = 1;
};
