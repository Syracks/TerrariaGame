#include "SettingsMenu.hpp"
#include "core/Constants.hpp"

SettingsMenu::SettingsMenu() = default;

SettingsMenu::Action SettingsMenu::update() {
    if (IsKeyPressed(KEY_ESCAPE) || IsKeyPressed(KEY_ENTER)) {
        return Action::Back;
    }
    return Action::None;
}

void SettingsMenu::render() const {
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{20, 20, 30, 255});

    const char* title = "Settings";
    int titleSize = 50;
    int titleWidth = MeasureText(title, titleSize);
    DrawText(title, (constants::SCREEN_WIDTH - titleWidth) / 2, 100, titleSize, GREEN);

    int y = 220;
    int lineHeight = 30;

    DrawText("Mouse sensitivity, key bindings, and", 100, y, 20, LIGHTGRAY);
    y += lineHeight;
    DrawText("other options would go here.", 100, y, 20, LIGHTGRAY);

    y = 350;
    Color backColor = WHITE;
    const char* backText = "Press ENTER to go back";
    int backWidth = MeasureText(backText, 22);
    DrawText(backText, (constants::SCREEN_WIDTH - backWidth) / 2, y, 22, backColor);
}
