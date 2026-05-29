#include "InventoryScreen.hpp"
#include "entities/Player.hpp"
#include "items/Inventory.hpp"
#include "world/TileRegistry.hpp"
#include "core/TextureManager.hpp"
#include "core/SoundManager.hpp"
#include "core/Constants.hpp"
#include "crafting/Recipe.hpp"

#include <string>

namespace {
    constexpr int SLOT_SIZE = 50;
    constexpr int SLOT_MARGIN = 4;
    constexpr int GRID_COLS = 5;
    constexpr int GRID_Y = 80;
    constexpr int PANEL_X = 15;
    constexpr int PANEL_Y = 320;
    constexpr int PANEL_W = 380;
    constexpr int PANEL_H = 350;
    constexpr int ROW_H = 40;
    constexpr int VISIBLE_ROWS = 8;

    constexpr int MENU_BTN_W = 200;
    constexpr int MENU_BTN_H = 40;
    constexpr int MENU_BTN_X = constants::SCREEN_WIDTH - MENU_BTN_W - 15;
    constexpr int MENU_BTN_Y = constants::SCREEN_HEIGHT - MENU_BTN_H - 15;
}

Rectangle InventoryScreen::getSlotRect(int index) const {
    int gridW = GRID_COLS * SLOT_SIZE + (GRID_COLS - 1) * SLOT_MARGIN;
    int gridX = (constants::SCREEN_WIDTH - gridW) / 2;
    int col = index % GRID_COLS;
    int row = index / GRID_COLS;
    return {
        static_cast<float>(gridX + col * (SLOT_SIZE + SLOT_MARGIN)),
        static_cast<float>(GRID_Y + row * (SLOT_SIZE + SLOT_MARGIN)),
        static_cast<float>(SLOT_SIZE),
        static_cast<float>(SLOT_SIZE)
    };
}

int InventoryScreen::getSlotAt(Vector2 mouse) const {
    for (int i = 0; i < constants::INVENTORY_SLOTS; ++i) {
        if (CheckCollisionPointRec(mouse, getSlotRect(i)))
            return i;
    }
    return -1;
}

InventoryScreen::Action InventoryScreen::update(Player& player) {
    auto& inventory = player.getInventory();
    auto& slots = inventory.getSlots();
    Vector2 mouse = GetMousePosition();

    m_hoveredSlot = getSlotAt(mouse);

    int wheel = GetMouseWheelMove();
    if (wheel != 0) {
        m_scrollOffset -= static_cast<int>(wheel);
        const auto& recipes = RecipeDatabase::getAvailable(slots.begin(), slots.end());
        int maxOffset = static_cast<int>(recipes.size()) - VISIBLE_ROWS;
        if (maxOffset < 0) maxOffset = 0;
        if (m_scrollOffset < 0) m_scrollOffset = 0;
        if (m_scrollOffset > maxOffset) m_scrollOffset = maxOffset;
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_RIGHT) && m_draggedStack.tileId != TileId::Air) {
        m_draggedStack = {TileId::Air, 0};
    }

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
        Rectangle menuBtn = {
            static_cast<float>(MENU_BTN_X),
            static_cast<float>(MENU_BTN_Y),
            static_cast<float>(MENU_BTN_W),
            static_cast<float>(MENU_BTN_H)
        };
        if (CheckCollisionPointRec(mouse, menuBtn)) {
            return Action::ReturnToMenu;
        }

        int slotIdx = getSlotAt(mouse);
        if (slotIdx >= 0) {
            if (m_draggedStack.tileId == TileId::Air) {
                if (slots[slotIdx].tileId != TileId::Air) {
                    m_draggedStack = slots[slotIdx];
                    slots[slotIdx] = {TileId::Air, 0};
                }
            } else {
                if (slots[slotIdx].tileId == TileId::Air) {
                    slots[slotIdx] = m_draggedStack;
                    m_draggedStack = {TileId::Air, 0};
                } else if (slots[slotIdx].tileId == m_draggedStack.tileId) {
                    slots[slotIdx].count += m_draggedStack.count;
                    m_draggedStack = {TileId::Air, 0};
                } else {
                    ItemStack temp = slots[slotIdx];
                    slots[slotIdx] = m_draggedStack;
                    m_draggedStack = temp;
                }
            }
            return Action::None;
        }

        const auto& recipes = RecipeDatabase::getAvailable(slots.begin(), slots.end());
        for (size_t i = 0; i < recipes.size(); ++i) {
            int listY = PANEL_Y + 20 + static_cast<int>(i - m_scrollOffset) * ROW_H;
            if (listY < PANEL_Y + 20) continue;
            if (listY + ROW_H > PANEL_Y + PANEL_H) break;
            Rectangle rowRect = {
                static_cast<float>(PANEL_X + 5),
                static_cast<float>(listY),
                static_cast<float>(PANEL_W - 10),
                static_cast<float>(ROW_H)
            };
            if (CheckCollisionPointRec(mouse, rowRect)) {
                const Recipe* recipe = recipes[i];
                for (const auto& ing : recipe->ingredients) {
                    inventory.removeItem(ing.item, ing.count);
                }
                inventory.addItem(recipe->result, recipe->resultCount);
                SoundManager::instance().play(SoundManager::Craft);
                break;
            }
        }
    }

    return Action::None;
}

void InventoryScreen::render(const Player& player) const {
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT,
                  Color{0, 0, 0, 140});

    const auto& inventory = player.getInventory();
    const auto& slots = inventory.getSlots();
    auto& texMgr = TextureManager::instance();

    int gridW = GRID_COLS * SLOT_SIZE + (GRID_COLS - 1) * SLOT_MARGIN;
    int gridX = (constants::SCREEN_WIDTH - gridW) / 2;

    DrawText("Inventory", gridX, GRID_Y - 25, 20, WHITE);

    for (int i = 0; i < constants::INVENTORY_SLOTS; ++i) {
        int col = i % GRID_COLS;
        int row = i / GRID_COLS;
        int x = gridX + col * (SLOT_SIZE + SLOT_MARGIN);
        int y = GRID_Y + row * (SLOT_SIZE + SLOT_MARGIN);
        Color bg = (i < constants::HOTBAR_SLOTS) ? Color{50, 50, 60, 220} : Color{40, 40, 50, 200};
        DrawRectangle(x, y, SLOT_SIZE, SLOT_SIZE, bg);

        Color border = Color{80, 80, 100, 255};
        if (i == m_hoveredSlot)
            border = Color{180, 180, 220, 255};
        DrawRectangleLines(x, y, SLOT_SIZE, SLOT_SIZE, border);

        if (slots[i].tileId != TileId::Air && slots[i].count > 0) {
            const auto& def = TileRegistry::instance().get(slots[i].tileId);
            const Texture2D& tex = texMgr.getTexture(slots[i].tileId);
            int iconSize = 28;
            int iconX = x + (SLOT_SIZE - iconSize) / 2;
            int iconY = y + 4;
            if (tex.id > 0) {
                float scale = static_cast<float>(iconSize) / tex.width;
                DrawTextureEx(tex, {static_cast<float>(iconX), static_cast<float>(iconY)}, 0.0f, scale, WHITE);
            } else {
                DrawRectangle(iconX, iconY, iconSize, iconSize, def.color);
            }
            std::string countText = std::to_string(slots[i].count);
            DrawText(countText.c_str(), x + 5, y + SLOT_SIZE - 18, 12, WHITE);
        }
    }

    DrawRectangle(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, Color{20, 20, 30, 220});
    DrawRectangleLines(PANEL_X, PANEL_Y, PANEL_W, PANEL_H, Color{80, 80, 100, 255});
    DrawText("Crafting", PANEL_X + 10, PANEL_Y + 5, 16, WHITE);

    const auto& recipes = RecipeDatabase::getAvailable(slots.begin(), slots.end());

    int textY = PANEL_Y + 25;
    int startIdx = m_scrollOffset;
    int endIdx = startIdx + VISIBLE_ROWS;
    if (endIdx > static_cast<int>(recipes.size())) endIdx = recipes.size();

    for (int i = startIdx; i < endIdx; ++i) {
        const Recipe* r = recipes[i];
        int ry = PANEL_Y + 20 + (i - startIdx) * ROW_H;
        DrawRectangle(PANEL_X + 5, ry, PANEL_W - 10, ROW_H - 2, Color{30, 30, 40, 200});

        TileId res = r->result;
        const auto& def = TileRegistry::instance().get(res);
        const Texture2D& tex = texMgr.getTexture(res);
        int iconSize = 28;
        if (tex.id > 0) {
            float scale = static_cast<float>(iconSize) / tex.width;
            DrawTextureEx(tex, {static_cast<float>(PANEL_X + 8), static_cast<float>(ry + 4)}, 0.0f, scale, WHITE);
        } else {
            DrawRectangle(PANEL_X + 8, ry + 4, iconSize, iconSize, def.color);
        }

        std::string resultLabel = def.name + " x" + std::to_string(r->resultCount);
        DrawText(resultLabel.c_str(), PANEL_X + 42, ry + 4, 14, WHITE);

        std::string ingredients;
        for (size_t j = 0; j < r->ingredients.size(); ++j) {
            if (j > 0) ingredients += ", ";
            ingredients += TileRegistry::instance().get(r->ingredients[j].item).name;
            ingredients += " x" + std::to_string(r->ingredients[j].count);
        }
        DrawText(ingredients.c_str(), PANEL_X + 42, ry + 22, 11, Color{160, 160, 180, 255});
    }

    if (recipes.empty()) {
        DrawText("No craftable items", PANEL_X + 10, PANEL_Y + 35, 14, Color{120, 120, 140, 255});
    }

    if (m_scrollOffset > 0) {
        DrawText("^", PANEL_X + PANEL_W / 2 - 5, PANEL_Y + PANEL_H - 18, 14, Color{160, 160, 180, 255});
    }
    if (m_scrollOffset + VISIBLE_ROWS < static_cast<int>(recipes.size())) {
        DrawText("v", PANEL_X + PANEL_W / 2 - 5, PANEL_Y + PANEL_H - 18, 14, Color{160, 160, 180, 255});
    }

    Vector2 mouse = GetMousePosition();
    Rectangle menuBtn = {
        static_cast<float>(MENU_BTN_X),
        static_cast<float>(MENU_BTN_Y),
        static_cast<float>(MENU_BTN_W),
        static_cast<float>(MENU_BTN_H)
    };
    bool hover = CheckCollisionPointRec(mouse, menuBtn);
    DrawRectangle(MENU_BTN_X, MENU_BTN_Y, MENU_BTN_W, MENU_BTN_H,
                  hover ? Color{80, 40, 40, 220} : Color{50, 30, 30, 200});
    DrawRectangleLines(MENU_BTN_X, MENU_BTN_Y, MENU_BTN_W, MENU_BTN_H,
                       hover ? Color{200, 80, 80, 255} : Color{120, 60, 60, 255});
    const char* label = "Return to Menu";
    int textW = MeasureText(label, 16);
    DrawText(label, MENU_BTN_X + (MENU_BTN_W - textW) / 2, MENU_BTN_Y + 12, 16, WHITE);

    if (m_draggedStack.tileId != TileId::Air && m_draggedStack.count > 0) {
        int iconSize = 32;
        int iconX = static_cast<int>(mouse.x) - iconSize / 2;
        int iconY = static_cast<int>(mouse.y) - iconSize / 2;
        const auto& def = TileRegistry::instance().get(m_draggedStack.tileId);
        const Texture2D& tex = texMgr.getTexture(m_draggedStack.tileId);
        if (tex.id > 0) {
            float scale = static_cast<float>(iconSize) / tex.width;
            DrawTextureEx(tex, {static_cast<float>(iconX), static_cast<float>(iconY)}, 0.0f, scale, WHITE);
        } else {
            DrawRectangle(iconX, iconY, iconSize, iconSize, def.color);
        }
        std::string countText = std::to_string(m_draggedStack.count);
        DrawText(countText.c_str(), iconX + 4, iconY + iconSize - 16, 12, WHITE);
        DrawRectangleLines(iconX, iconY, iconSize, iconSize, Color{200, 200, 220, 200});
    }
}
