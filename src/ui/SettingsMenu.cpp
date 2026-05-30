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

    bool isClicked(Rectangle rect) {
        if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;
        return CheckCollisionPointRec(GetMousePosition(), rect);
    }
}

SettingsMenu::SettingsMenu() = default;

SettingsMenu::Action SettingsMenu::update() {
    Vector2 mouse = GetMousePosition();

    Rectangle backRect = {
        (constants::SCREEN_WIDTH - 160.0f) / 2.0f,
        static_cast<float>(BACK_Y),
        160.0f,
        44.0f
    };

    if (IsKeyPressed(KEY_ESCAPE) || isClicked(backRect)) {
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
        if (CheckCollisionPointRec(mouse, sliderTrack)) {
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

    Rectangle backRect = {
        (constants::SCREEN_WIDTH - 160.0f) / 2.0f,
        static_cast<float>(BACK_Y),
        160.0f,
        44.0f
    };
    bool backHov = CheckCollisionPointRec(GetMousePosition(), backRect);
    DrawRectangleRec(backRect, backHov ? Color{50, 45, 45, 255} : Color{35, 35, 35, 255});
    DrawRectangleLinesEx(backRect, 1, backHov ? Color{180, 100, 100, 255} : Color{60, 60, 60, 255});
    const char* backText = "Back";
    DrawText(backText, backRect.x + (160 - MeasureText(backText, 24)) / 2,
             backRect.y + 10, 24, backHov ? WHITE : GRAY);
}
