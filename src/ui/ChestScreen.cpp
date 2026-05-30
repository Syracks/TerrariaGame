#include "ChestScreen.hpp"
#include "entities/Player.hpp"
#include "items/Inventory.hpp"
#include "items/ItemDefinition.hpp"
#include "world/TileRegistry.hpp"
#include "core/Constants.hpp"
#include "core/TextureManager.hpp"

static constexpr int SLOT_SIZE = 50;
static constexpr int SLOT_MARGIN = 4;
static constexpr int GRID_COLS = 9;
static constexpr int GRID_Y = 80;

static int gridWidth(int cols) {
    return cols * SLOT_SIZE + (cols - 1) * SLOT_MARGIN;
}

static int gridX(int cols) {
    return (constants::SCREEN_WIDTH - gridWidth(cols)) / 2;
}

void ChestScreen::open(int chestTX, int chestTY) {
    m_chestTX = chestTX;
    m_chestTY = chestTY;
    m_open = true;
}

void ChestScreen::close() {
    m_open = false;
}

Rectangle ChestScreen::getPlayerSlotRect(int index) const {
    int col = index % GRID_COLS;
    int row = index / GRID_COLS;
    return {
        static_cast<float>(gridX(GRID_COLS) + col * (SLOT_SIZE + SLOT_MARGIN)),
        static_cast<float>(GRID_Y + row * (SLOT_SIZE + SLOT_MARGIN)),
        static_cast<float>(SLOT_SIZE),
        static_cast<float>(SLOT_SIZE)
    };
}

Rectangle ChestScreen::getChestSlotRect(int index) const {
    int col = index % CHEST_GRID_W;
    int row = index / CHEST_GRID_W;
    int panelY = GRID_Y + 5 * (SLOT_SIZE + SLOT_MARGIN) + 50;
    int panelX = gridX(CHEST_GRID_W);
    return {
        static_cast<float>(panelX + col * (SLOT_SIZE + SLOT_MARGIN)),
        static_cast<float>(panelY + row * (SLOT_SIZE + SLOT_MARGIN)),
        static_cast<float>(SLOT_SIZE),
        static_cast<float>(SLOT_SIZE)
    };
}

int ChestScreen::getSlotAt(Vector2 mouse) const {
    for (int i = 0; i < CHEST_SLOTS; ++i) {
        if (CheckCollisionPointRec(mouse, getChestSlotRect(i)))
            return i;
    }
    for (int i = 0; i < constants::INVENTORY_SLOTS; ++i) {
        if (CheckCollisionPointRec(mouse, getPlayerSlotRect(i)))
            return CHEST_SLOTS + i;
    }
    return -1;
}

void ChestScreen::update(Player& player, World& world) {
    if (!m_open) return;

    auto mouse = GetMousePosition();
    m_hoveredSlot = getSlotAt(mouse);

    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && m_hoveredSlot >= 0) {
        auto& chestInv = world.getChest(m_chestTX, m_chestTY);
        auto& playerSlots = player.getInventory().getSlots();

        if (m_hoveredSlot < CHEST_SLOTS) {
            auto& src = chestInv[m_hoveredSlot];
            if (src.count > 0) {
                TileId id = src.tileId;
                int maxStack = ItemDatabase::instance().getMaxStack(id);
                int toMove = src.count;
                for (auto& ps : playerSlots) {
                    if (ps.tileId == id && ps.count < maxStack) {
                        int space = maxStack - ps.count;
                        int take = toMove < space ? toMove : space;
                        ps.count += take;
                        toMove -= take;
                        if (toMove <= 0) break;
                    }
                }
                if (toMove > 0) {
                    for (auto& ps : playerSlots) {
                        if (ps.tileId == TileId::Air) {
                            int take = toMove < maxStack ? toMove : maxStack;
                            ps = {id, take};
                            toMove -= take;
                            break;
                        }
                    }
                }
                int moved = src.count - toMove;
                src.count -= moved;
                if (src.count <= 0) src = {TileId::Air, 0};
            }
        } else {
            int pIdx = m_hoveredSlot - CHEST_SLOTS;
            auto& ps = playerSlots[pIdx];
            if (ps.count > 0) {
                TileId id = ps.tileId;
                int maxStack = ItemDatabase::instance().getMaxStack(id);
                int toMove = ps.count;
                for (auto& cs : chestInv) {
                    if (cs.tileId == id && cs.count < maxStack) {
                        int space = maxStack - cs.count;
                        int take = toMove < space ? toMove : space;
                        cs.count += take;
                        toMove -= take;
                        if (toMove <= 0) break;
                    }
                }
                if (toMove > 0) {
                    for (auto& cs : chestInv) {
                        if (cs.tileId == TileId::Air) {
                            int take = toMove < maxStack ? toMove : maxStack;
                            cs = {id, take};
                            toMove -= take;
                            break;
                        }
                    }
                }
                int moved = ps.count - toMove;
                ps.count -= moved;
                if (ps.count <= 0) ps = {TileId::Air, 0};
            }
        }
    }
}

void ChestScreen::render(const Player& player, const World& world) const {
    if (!m_open) return;

    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{0, 0, 0, 140});

    const auto& slots = player.getInventory().getSlots();
    auto& texMgr = TextureManager::instance();
    auto& itemDb = ItemDatabase::instance();
    auto& reg = TileRegistry::instance();

    int invGridX = gridX(GRID_COLS);
    DrawText("Inventory", invGridX, GRID_Y - 25, 20, WHITE);

    for (int i = 0; i < constants::INVENTORY_SLOTS; ++i) {
        Rectangle r = getPlayerSlotRect(i);
        Color bg = (i < constants::HOTBAR_SLOTS) ? Color{50, 50, 60, 220} : Color{40, 40, 50, 200};
        DrawRectangleRec(r, bg);

        Color border = Color{80, 80, 100, 255};
        if (m_hoveredSlot == CHEST_SLOTS + i)
            border = Color{180, 180, 220, 255};
        DrawRectangleLinesEx(r, 1, border);

        if (slots[i].tileId != TileId::Air && slots[i].count > 0) {
            const Texture2D& tex = texMgr.getTexture(slots[i].tileId);
            int iconSize = 28;
            int iconX = static_cast<int>(r.x) + (SLOT_SIZE - iconSize) / 2;
            int iconY = static_cast<int>(r.y) + 4;
            if (tex.id > 0) {
                float scale = static_cast<float>(iconSize) / tex.width;
                DrawTextureEx(tex, {static_cast<float>(iconX), static_cast<float>(iconY)}, 0.0f, scale, WHITE);
            } else {
                DrawRectangle(iconX, iconY, iconSize, iconSize, reg.get(slots[i].tileId).color);
            }
            int maxStack = itemDb.getMaxStack(slots[i].tileId);
            std::string countText = std::to_string(slots[i].count) + "/" + std::to_string(maxStack);
            DrawText(countText.c_str(), static_cast<int>(r.x) + 5, static_cast<int>(r.y) + SLOT_SIZE - 18, 10, Color{200, 200, 200, 200});
        }
    }

    int panelY = GRID_Y + 5 * (SLOT_SIZE + SLOT_MARGIN) + 50;
    int panelW = gridWidth(CHEST_GRID_W) + 20;
    int panelH = CHEST_GRID_H * (SLOT_SIZE + SLOT_MARGIN) - SLOT_MARGIN + 40;
    int panelX = gridX(CHEST_GRID_W) - 10;

    DrawRectangle(panelX, panelY, panelW, panelH, Color{20, 20, 30, 220});
    DrawRectangleLines(panelX, panelY, panelW, panelH, Color{80, 80, 100, 255});

    const char* chestLabel = "Chest";
    DrawText(chestLabel, panelX + 10, panelY + 8, 16, WHITE);

    const auto& chestInv = world.getChestConst(m_chestTX, m_chestTY);
    for (int i = 0; i < CHEST_SLOTS; ++i) {
        Rectangle r = getChestSlotRect(i);
        Color bg = Color{50, 50, 60, 220};
        DrawRectangleRec(r, bg);

        Color border = Color{80, 80, 100, 255};
        if (m_hoveredSlot == i)
            border = Color{180, 180, 220, 255};
        DrawRectangleLinesEx(r, 1, border);

        if (chestInv[i].tileId != TileId::Air && chestInv[i].count > 0) {
            const Texture2D& tex = texMgr.getTexture(chestInv[i].tileId);
            int iconSize = 28;
            int iconX = static_cast<int>(r.x) + (SLOT_SIZE - iconSize) / 2;
            int iconY = static_cast<int>(r.y) + 4;
            if (tex.id > 0) {
                float scale = static_cast<float>(iconSize) / tex.width;
                DrawTextureEx(tex, {static_cast<float>(iconX), static_cast<float>(iconY)}, 0.0f, scale, WHITE);
            } else {
                DrawRectangle(iconX, iconY, iconSize, iconSize, reg.get(chestInv[i].tileId).color);
            }
            int maxStack = itemDb.getMaxStack(chestInv[i].tileId);
            std::string countText = std::to_string(chestInv[i].count) + "/" + std::to_string(maxStack);
            DrawText(countText.c_str(), static_cast<int>(r.x) + 5, static_cast<int>(r.y) + SLOT_SIZE - 18, 10, Color{200, 200, 200, 200});
        }
    }
}
