#include "SettingsMenu.hpp"
#include "core/Constants.hpp"
#include "core/Math.hpp"
#include "core/SoundManager.hpp"

#include <algorithm>
#include <string>
#include <fstream>
#include <iostream>

namespace {
    constexpr int SLIDER_X = 350;
    constexpr int SLIDER_W = 400;
    constexpr int SLIDER_H = 12;
    constexpr int HANDLE_R = 10;

    constexpr int COL1_X = 350;
    constexpr int COL2_X = 550;
    constexpr int ROW_H = 60;
    constexpr int START_Y = 180;

    bool isClicked(Rectangle rect) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;
        return CheckCollisionPointRec(math::getVirtualMouse(), rect);
    }
}

SettingsMenu::SettingsMenu() {
    loadFromFile();
}

SettingsMenu::Action SettingsMenu::update() {
    Vector2 mouse = math::getVirtualMouse();

    Rectangle backRect = {
        (constants::SCREEN_WIDTH - 160.0f) / 2.0f,
        520.0f,
        160.0f,
        44.0f
    };

    if (IsKeyPressed(KEY_ESCAPE) || isClicked(backRect)) {
        saveToFile();
        return Action::Back;
    }

    Rectangle sliderTrack = {
        static_cast<float>(SLIDER_X),
        static_cast<float>(START_Y + ROW_H * 0),
        static_cast<float>(SLIDER_W),
        static_cast<float>(SLIDER_H)
    };

    Rectangle fullscreenRect = {
        static_cast<float>(COL2_X), static_cast<float>(START_Y + ROW_H * 1),
        160.0f, 30.0f
    };

    Rectangle minimapRect = {
        static_cast<float>(COL2_X), static_cast<float>(START_Y + ROW_H * 2),
        160.0f, 30.0f
    };

    Rectangle prevResRect = {
        static_cast<float>(COL1_X), static_cast<float>(START_Y + ROW_H * 3),
        30.0f, 30.0f
    };
    Rectangle nextResRect = {
        static_cast<float>(COL1_X + 200), static_cast<float>(START_Y + ROW_H * 3),
        30.0f, 30.0f
    };

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP)) {
        m_volume = std::min(1.0f, m_volume + 0.05f);
        SoundManager::instance().setMasterVolume(m_volume);
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN)) {
        m_volume = std::max(0.0f, m_volume - 0.05f);
        SoundManager::instance().setMasterVolume(m_volume);
    }

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        if (CheckCollisionPointRec(mouse, sliderTrack)) {
            m_volume = std::clamp((mouse.x - SLIDER_X) / static_cast<float>(SLIDER_W), 0.0f, 1.0f);
            SoundManager::instance().setMasterVolume(m_volume);
        }
    }

    if (isClicked(fullscreenRect)) {
        m_fullscreen = !m_fullscreen;
        int monitor = GetCurrentMonitor();
        if (m_fullscreen) {
            SetWindowSize(GetMonitorWidth(monitor), GetMonitorHeight(monitor));
            ToggleFullscreen();
        } else {
            ToggleFullscreen();
            int w = constants::RESOLUTIONS[m_resolutionIndex][0];
            int h = constants::RESOLUTIONS[m_resolutionIndex][1];
            SetWindowSize(w, h);
        }
    }

    if (isClicked(minimapRect)) {
        m_showMinimap = !m_showMinimap;
    }

    if (isClicked(prevResRect)) {
        m_resolutionIndex = (m_resolutionIndex - 1 + constants::RESOLUTION_COUNT) % constants::RESOLUTION_COUNT;
        if (!m_fullscreen) {
            SetWindowSize(constants::RESOLUTIONS[m_resolutionIndex][0],
                          constants::RESOLUTIONS[m_resolutionIndex][1]);
        }
    }
    if (isClicked(nextResRect)) {
        m_resolutionIndex = (m_resolutionIndex + 1) % constants::RESOLUTION_COUNT;
        if (!m_fullscreen) {
            SetWindowSize(constants::RESOLUTIONS[m_resolutionIndex][0],
                          constants::RESOLUTIONS[m_resolutionIndex][1]);
        }
    }

    return Action::None;
}

void SettingsMenu::render() const {
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{20, 20, 30, 255});

    const char* title = "Settings";
    int titleSize = 50;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (constants::SCREEN_WIDTH - titleWidth) / 2, 60, titleSize, GREEN);

    int row = 0;

    DrawText("Volume", COL1_X, START_Y + ROW_H * row - 24, 22, LIGHTGRAY);
    DrawRectangle(SLIDER_X, START_Y + ROW_H * row, SLIDER_W, SLIDER_H, Color{50, 50, 60, 255});
    int fillW = static_cast<int>(SLIDER_W * m_volume);
    if (fillW > 0)
        DrawRectangle(SLIDER_X, START_Y + ROW_H * row, fillW, SLIDER_H, GREEN);
    DrawRectangleLines(SLIDER_X, START_Y + ROW_H * row, SLIDER_W, SLIDER_H, Color{80, 80, 100, 255});
    float handleX = SLIDER_X + m_volume * SLIDER_W;
    DrawCircle(static_cast<int>(handleX), START_Y + ROW_H * row + SLIDER_H / 2, HANDLE_R, WHITE);
    DrawCircle(static_cast<int>(handleX), START_Y + ROW_H * row + SLIDER_H / 2, HANDLE_R - 2, Color{200, 200, 220, 255});
    std::string pct = std::to_string(static_cast<int>(m_volume * 100)) + "%";
    DrawText(pct.c_str(), SLIDER_X + SLIDER_W + 20, START_Y + ROW_H * row - 4, 20, WHITE);
    row++;

    DrawText("Fullscreen", COL1_X, START_Y + ROW_H * row - 24, 22, LIGHTGRAY);
    Rectangle fullscreenRect = {
        static_cast<float>(COL2_X), static_cast<float>(START_Y + ROW_H * row),
        160.0f, 30.0f
    };
    bool fsHov = CheckCollisionPointRec(math::getVirtualMouse(), fullscreenRect);
    Color fsBg = m_fullscreen ? (fsHov ? Color{60, 90, 60, 255} : Color{40, 70, 40, 255})
                              : (fsHov ? Color{60, 50, 50, 255} : Color{45, 35, 35, 255});
    Color fsBorder = m_fullscreen ? (fsHov ? Color{100, 200, 100, 255} : Color{60, 120, 60, 255})
                                  : (fsHov ? Color{180, 100, 100, 255} : Color{80, 60, 60, 255});
    DrawRectangleRec(fullscreenRect, fsBg);
    DrawRectangleLinesEx(fullscreenRect, 1, fsBorder);
    const char* fsText = m_fullscreen ? "On" : "Off";
    DrawText(fsText, COL2_X + (160 - MeasureText(fsText, 22)) / 2,
             START_Y + ROW_H * row + 4, 22, WHITE);
    row++;

    DrawText("Show Minimap", COL1_X, START_Y + ROW_H * row - 24, 22, LIGHTGRAY);
    Rectangle minimapRect = {
        static_cast<float>(COL2_X), static_cast<float>(START_Y + ROW_H * row),
        160.0f, 30.0f
    };
    bool mmHov = CheckCollisionPointRec(math::getVirtualMouse(), minimapRect);
    Color mmBg = m_showMinimap ? (mmHov ? Color{60, 90, 60, 255} : Color{40, 70, 40, 255})
                               : (mmHov ? Color{60, 50, 50, 255} : Color{45, 35, 35, 255});
    Color mmBorder = m_showMinimap ? (mmHov ? Color{100, 200, 100, 255} : Color{60, 120, 60, 255})
                                   : (mmHov ? Color{180, 100, 100, 255} : Color{80, 60, 60, 255});
    DrawRectangleRec(minimapRect, mmBg);
    DrawRectangleLinesEx(minimapRect, 1, mmBorder);
    const char* mmText = m_showMinimap ? "On" : "Off";
    DrawText(mmText, COL2_X + (160 - MeasureText(mmText, 22)) / 2,
             START_Y + ROW_H * row + 4, 22, WHITE);
    row++;

    DrawText("Resolution", COL1_X, START_Y + ROW_H * row - 24, 22, LIGHTGRAY);
    std::string resText = std::to_string(constants::RESOLUTIONS[m_resolutionIndex][0]) + "x" +
                          std::to_string(constants::RESOLUTIONS[m_resolutionIndex][1]);
    Rectangle prevResRect = {
        static_cast<float>(COL1_X), static_cast<float>(START_Y + ROW_H * row),
        30.0f, 30.0f
    };
    Rectangle nextResRect = {
        static_cast<float>(COL1_X + 200), static_cast<float>(START_Y + ROW_H * row),
        30.0f, 30.0f
    };
    bool prevHov = CheckCollisionPointRec(math::getVirtualMouse(), prevResRect);
    bool nextHov = CheckCollisionPointRec(math::getVirtualMouse(), nextResRect);
    DrawRectangleRec(prevResRect, prevHov ? Color{60, 60, 80, 255} : Color{40, 40, 55, 255});
    DrawRectangleLinesEx(prevResRect, 1, prevHov ? WHITE : Color{80, 80, 100, 255});
    DrawText("<", COL1_X + 9, START_Y + ROW_H * row + 4, 22, WHITE);
    DrawText(resText.c_str(), COL1_X + 40, START_Y + ROW_H * row + 4, 22, WHITE);
    DrawRectangleRec(nextResRect, nextHov ? Color{60, 60, 80, 255} : Color{40, 40, 55, 255});
    DrawRectangleLinesEx(nextResRect, 1, nextHov ? WHITE : Color{80, 80, 100, 255});
    DrawText(">", COL1_X + 209, START_Y + ROW_H * row + 4, 22, WHITE);

    Rectangle backRect = {
        (constants::SCREEN_WIDTH - 160.0f) / 2.0f,
        520.0f,
        160.0f,
        44.0f
    };
    bool backHov = CheckCollisionPointRec(math::getVirtualMouse(), backRect);
    DrawRectangleRec(backRect, backHov ? Color{50, 45, 45, 255} : Color{35, 35, 35, 255});
    DrawRectangleLinesEx(backRect, 1, backHov ? Color{180, 100, 100, 255} : Color{60, 60, 60, 255});
    const char* backText = "Back";
    DrawText(backText, backRect.x + (160 - MeasureText(backText, 24)) / 2,
             backRect.y + 10, 24, backHov ? WHITE : GRAY);
}

void SettingsMenu::saveToFile() const {
    std::ofstream file("config/settings.txt");
    if (!file.is_open()) return;
    file << m_volume << "\n";
    file << m_resolutionIndex << "\n";
    file << (m_fullscreen ? "1" : "0") << "\n";
    file << (m_showMinimap ? "1" : "0") << "\n";
}

void SettingsMenu::loadFromFile() {
    std::ifstream file("config/settings.txt");
    if (!file.is_open()) return;
    std::string line;
    if (std::getline(file, line)) {
        try {
            float v = std::stof(line);
            if (!std::isnan(v)) m_volume = std::clamp(v, 0.0f, 1.0f);
        } catch (...) {}
    }
    if (std::getline(file, line)) {
        try {
            int idx = std::stoi(line);
            if (idx >= 0 && idx < constants::RESOLUTION_COUNT) m_resolutionIndex = idx;
        } catch (...) {}
    }
    if (std::getline(file, line)) {
        m_fullscreen = (line == "1");
    }
    if (std::getline(file, line)) {
        m_showMinimap = (line != "0");
    }
}
