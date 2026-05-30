#include "HUD.hpp"
#include "entities/Player.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDefinition.hpp"
#include "world/TileRegistry.hpp"
#include "core/Constants.hpp"
#include "core/TextureManager.hpp"
#include <raylib.h>
#include <string>

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

void HUD::render(const Player& player) {
    renderPlayerHP(player);
    const auto& inventory = player.getInventory();
    const auto& slots = inventory.getSlots();
    int selected = inventory.getSelectedIndex();

    int slotSize = 50;
    int margin = 4;
    int totalWidth = constants::HOTBAR_SLOTS * slotSize + (constants::HOTBAR_SLOTS - 1) * margin;
    int startX = (constants::SCREEN_WIDTH - totalWidth) / 2;
    int startY = constants::SCREEN_HEIGHT - slotSize - 10;

    auto& texMgr = TextureManager::instance();
    auto& itemDb = ItemDatabase::instance();

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
    DrawText(selectedName.c_str(), 10, startY - 25, 16, WHITE);
}
