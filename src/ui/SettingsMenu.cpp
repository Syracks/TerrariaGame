#include "SettingsMenu.hpp"
#include "core/Constants.hpp"
#include "core/SoundManager.hpp"

#include <algorithm>
#include <string>

namespace {
    constexpr int SLIDER_X = 350;
    constexpr int SLIDER_W = 400;
    constexpr int SLIDER_H = 12;
    constexpr int HANDLE_R = 10;
    constexpr int VOLUME_Y = 220;
    constexpr int BACK_Y = 380;
}

SettingsMenu::SettingsMenu() = default;

SettingsMenu::Action SettingsMenu::update() {
    Vector2 mouse = GetMousePosition();

    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        return Action::Back;
    }

    if (IsKeyPressed(KEY_RIGHT) || IsKeyPressed(KEY_UP)) {
        m_volume = std::min(1.0f, m_volume + 0.05f);
        SoundManager::instance().setMasterVolume(m_volume);
    }
    if (IsKeyPressed(KEY_LEFT) || IsKeyPressed(KEY_DOWN)) {
        m_volume = std::max(0.0f, m_volume - 0.05f);
        SoundManager::instance().setMasterVolume(m_volume);
    }

    Rectangle sliderTrack = {
        static_cast<float>(SLIDER_X),
        static_cast<float>(VOLUME_Y),
        static_cast<float>(SLIDER_W),
        static_cast<float>(SLIDER_H)
    };

    if (IsMouseButtonDown(MOUSE_BUTTON_LEFT)) {
        float handleX = SLIDER_X + m_volume * SLIDER_W;
        Rectangle handleRect = {
            handleX - HANDLE_R,
            static_cast<float>(VOLUME_Y) - HANDLE_R,
            HANDLE_R * 2.0f,
            HANDLE_R * 2.0f
        };

        if (CheckCollisionPointRec(mouse, sliderTrack) ||
            CheckCollisionPointRec(mouse, handleRect)) {
            m_volume = std::clamp((mouse.x - SLIDER_X) / static_cast<float>(SLIDER_W), 0.0f, 1.0f);
            SoundManager::instance().setMasterVolume(m_volume);
        }
    }

    return Action::None;
}

void SettingsMenu::render() const {
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{20, 20, 30, 255});

    const char* title = "Settings";
    int titleSize = 50;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (constants::SCREEN_WIDTH - titleWidth) / 2, 100, titleSize, GREEN);

    DrawText("Volume", SLIDER_X, VOLUME_Y - 30, 22, LIGHTGRAY);

    DrawRectangle(SLIDER_X, VOLUME_Y, SLIDER_W, SLIDER_H, Color{50, 50, 60, 255});
    int fillW = static_cast<int>(SLIDER_W * m_volume);
    if (fillW > 0)
        DrawRectangle(SLIDER_X, VOLUME_Y, fillW, SLIDER_H, GREEN);
    DrawRectangleLines(SLIDER_X, VOLUME_Y, SLIDER_W, SLIDER_H, Color{80, 80, 100, 255});

    float handleX = SLIDER_X + m_volume * SLIDER_W;
    DrawCircle(static_cast<int>(handleX), VOLUME_Y + SLIDER_H / 2, HANDLE_R, WHITE);
    DrawCircle(static_cast<int>(handleX), VOLUME_Y + SLIDER_H / 2, HANDLE_R - 2, Color{200, 200, 220, 255});

    std::string pct = std::to_string(static_cast<int>(m_volume * 100)) + "%";
    DrawText(pct.c_str(), SLIDER_X + SLIDER_W + 20, VOLUME_Y - 4, 20, WHITE);

    DrawText("Use Arrow Keys or click & drag", SLIDER_X, VOLUME_Y + 30, 16, Color{120, 120, 140, 255});

    Color backColor = WHITE;
    const char* backText = "Press ENTER to go back";
    int backWidth = MeasureText(backText, 22);
    DrawText(backText, (constants::SCREEN_WIDTH - backWidth) / 2, BACK_Y, 22, backColor);
}
