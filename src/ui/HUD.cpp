#include "HUD.hpp"
#include "entities/Player.hpp"
#include "entities/Mob.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDefinition.hpp"
#include "items/Tool.hpp"
#include "world/TileRegistry.hpp"
#include "core/Constants.hpp"
#include "core/TextureManager.hpp"
#include <raylib.h>
#include <string>
#include <cmath>

HUD::Message HUD::s_message{};

constexpr float CYCLE_LENGTH = 420.0f;
constexpr int MSG_FONT_SIZE = 28;

void HUD::update(float dt) {
    if (s_message.timer > 0.0f) {
        s_message.timer -= dt;
    }
}

void HUD::showMessage(const std::string& text, float duration, int yOffset) {
    s_message.text = text;
    s_message.timer = duration;
    s_message.duration = duration;
    s_message.yOffset = yOffset;
}

void HUD::renderPlayerHP(const Player& player) {
    int barW = 200;
    int barH = 22;
    int x = constants::SCREEN_WIDTH - barW - 15;
    int y = 15;

    DrawRectangle(x, y, barW, barH, Color{30, 30, 30, 220});
    DrawRectangleLines(x, y, barW, barH, Color{80, 80, 80, 255});

    int hp = player.getHealth();
    int maxHp = player.getMaxHealth();
    int fillW = barW * hp / maxHp;
    Color hpColor = (hp > maxHp / 3) ? Color{60, 200, 60, 255} : Color{220, 50, 50, 255};
    DrawRectangle(x + 1, y + 1, fillW - 1, barH - 2, hpColor);

    std::string text = std::to_string(hp) + " / " + std::to_string(maxHp);
    int textW = MeasureText(text.c_str(), 14);
    DrawText(text.c_str(), x + (barW - textW) / 2, y + 4, 14, WHITE);
}

void HUD::render(const Player& player, float dayTime) {
    if (s_message.timer > 0.0f) {
        float alpha = 1.0f;
        if (s_message.timer < HUD::FADE_START) {
            alpha = s_message.timer / HUD::FADE_START;
        }
        unsigned char a = static_cast<unsigned char>(255 * alpha);
        Color shadow{0, 0, 0, static_cast<unsigned char>(180 * alpha)};
        Color text{180, 100, 255, a};
        int w = MeasureText(s_message.text.c_str(), MSG_FONT_SIZE);
        int x = (constants::SCREEN_WIDTH - w) / 2;
        int y = constants::SCREEN_HEIGHT / 2 - 60 + s_message.yOffset;
        DrawText(s_message.text.c_str(), x + 2, y + 2, MSG_FONT_SIZE, shadow);
        DrawText(s_message.text.c_str(), x, y, MSG_FONT_SIZE, text);
    }

    renderPlayerHP(player);
    renderTime(dayTime);
    const auto& inventory = player.getInventory();
    const auto& slots = inventory.getSlots();
    int selected = inventory.getSelectedIndex();

    int slotSize = 50;
    int margin = 4;
    int totalWidth = constants::HOTBAR_SLOTS * slotSize + (constants::HOTBAR_SLOTS - 1) * margin;
    int startX = (constants::SCREEN_WIDTH - totalWidth) / 2;
    int startY = constants::SCREEN_HEIGHT - slotSize - 10;

    auto& texMgr = TextureManager::instance();
    for (int i = 0; i < constants::HOTBAR_SLOTS; ++i) {
        int x = startX + i * (slotSize + margin);
        Color bg = (i == selected) ? Color{60, 60, 70, 255} : Color{40, 40, 50, 200};
        DrawRectangle(x, startY, slotSize, slotSize, bg);
        DrawRectangleLines(x, startY, slotSize, slotSize, (i == selected) ? WHITE : Color{100, 100, 120, 255});

        if (slots[i].tileId != TileId::Air && slots[i].count > 0) {
            auto& reg = TileRegistry::instance();
            const auto& def = reg.get(slots[i].tileId);

            int iconSize = 24;
            int iconX = x + (slotSize - iconSize) / 2;
            int iconY = startY + 5;

            const Texture2D& tex = texMgr.getTexture(slots[i].tileId);
            if (tex.id > 0) {
                float scale = static_cast<float>(iconSize) / tex.width;
                DrawTextureEx(tex, {static_cast<float>(iconX), static_cast<float>(iconY)}, 0.0f, scale, WHITE);
            } else {
                DrawRectangle(iconX, iconY, iconSize, iconSize, def.color);
            }

            std::string countText = std::to_string(slots[i].count);
            DrawText(countText.c_str(), x + 5, startY + slotSize - 20, 14, WHITE);
        }
    }

    auto& reg = TileRegistry::instance();
    const auto& selectedDef = reg.get(slots[selected].tileId);
    std::string selectedName = "Selected: " + selectedDef.name;
    if (isBow(slots[selected].tileId)) {
        int arrowCount = inventory.countItem(TileId::Arrow);
        selectedName += "  [Arrows: " + std::to_string(arrowCount) + "]";
    }
    DrawText(selectedName.c_str(), 10, startY - 25, 16, WHITE);
}

void HUD::renderBossHP(const Mob& boss) {
    int barW = 300;
    int barH = 18;
    int x = (constants::SCREEN_WIDTH - barW) / 2;
    int y = 45;

    DrawRectangle(x, y, barW, barH, Color{40, 20, 20, 220});
    DrawRectangleLines(x, y, barW, barH, Color{200, 60, 60, 255});

    int hp = boss.getHealth();
    int maxHp = boss.getMaxHealth();
    int fillW = barW * hp / maxHp;
    DrawRectangle(x + 1, y + 1, fillW - 1, barH - 2, Color{200, 50, 50, 255});

    std::string name = "Forest Guardian";
    int nameW = MeasureText(name.c_str(), 14);
    DrawText(name.c_str(), x + (barW - nameW) / 2, y - 16, 14, Color{255, 220, 180, 255});

    std::string hpText = std::to_string(hp) + " / " + std::to_string(maxHp);
    int textW = MeasureText(hpText.c_str(), 12);
    DrawText(hpText.c_str(), x + (barW - textW) / 2, y + 3, 12, WHITE);
}

void HUD::renderTime(float dayTime) {
    float t = dayTime / CYCLE_LENGTH;
    float totalHours = t * 24.0f + 8.0f;
    int hours = static_cast<int>(totalHours) % 24;
    int minutes = static_cast<int>((totalHours - std::floor(totalHours)) * 60.0f);

    std::string period = (hours >= 8 && hours < 20) ? "Day" : "Night";
    std::string timeStr = std::to_string(hours / 10) + std::to_string(hours % 10) + ":" +
                          std::to_string(minutes / 10) + std::to_string(minutes % 10) + " " + period;

    DrawText(timeStr.c_str(), 10, 15, 18, WHITE);
}
