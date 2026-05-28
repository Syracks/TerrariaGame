#include "MainMenu.hpp"
#include "core/Constants.hpp"
#include <algorithm>
#include <cctype>

MainMenu::MainMenu() {
    setVisible(true);
}

void MainMenu::setVisible(bool v) {
    m_visible = v;
    if (v) {
        m_screen = Screen::Main;
        m_selectedOption = 0;
        m_editingName = false;
        m_worldName.clear();
        refreshSlots();
    }
}

void MainMenu::reset() {
    m_screen = Screen::Main;
    m_selectedOption = 0;
    m_worldName.clear();
    m_editingName = false;
    m_worldSizeIndex = 1;
    m_selectedSlot = -1;
}

void MainMenu::refreshSlots() {
    m_slots = SaveManager::listSlots();
}

const char* MainMenu::worldSizeText(int index) {
    switch (index) {
        case 0: return "Small";
        case 1: return "Medium";
        case 2: return "Large";
        default: return "Medium";
    }
}

static bool isClicked(Rectangle rect) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;
    Vector2 mp = GetMousePosition();
    return CheckCollisionPointRec(mp, rect);
}

static bool isHovered(Rectangle rect) {
    Vector2 mp = GetMousePosition();
    return CheckCollisionPointRec(mp, rect);
}

MenuResult MainMenu::update() {
    MenuResult result;

    if (!m_visible) {
        result.type = MenuResult::None;
        return result;
    }

    Vector2 mousePos = GetMousePosition();
    bool mouseMoved = (mousePos.x != m_lastMousePos.x || mousePos.y != m_lastMousePos.y);
    m_lastMousePos = mousePos;

    if (m_screen == Screen::Main) {
        int optionSize = 30;
        Rectangle optionRects[4];
        for (int i = 0; i < m_mainOptionCount; ++i) {
            const char* labels[] = {"Create World", "Load World", "Settings", "Quit"};
            int w = MeasureText(labels[i], optionSize);
            int yPos = 260 + i * 60;
            optionRects[i] = {
                (constants::SCREEN_WIDTH - w) / 2.0f - 44,
                static_cast<float>(yPos),
                static_cast<float>(w + 48),
                static_cast<float>(optionSize + 8)
            };
        }

        for (int i = 0; i < m_mainOptionCount; ++i) {
            if (isClicked(optionRects[i])) {
                m_selectedOption = i;
                switch (i) {
                    case 0:
                        m_screen = Screen::NewGame;
                        m_selectedOption = 0;
                        m_worldName.clear();
                        m_editingName = true;
                        m_worldSizeIndex = 1;
                        break;
                    case 1:
                        refreshSlots();
                        m_screen = Screen::LoadGame;
                        m_selectedOption = 0;
                        break;
                    case 2:
                        result.type = MenuResult::Settings;
                        return result;
                    case 3:
                        result.type = MenuResult::Quit;
                        return result;
                }
                result.type = MenuResult::None;
                return result;
            }
        }

        if (mouseMoved) {
            for (int i = 0; i < m_mainOptionCount; ++i) {
                if (isHovered(optionRects[i])) {
                    m_selectedOption = i;
                    break;
                }
            }
        }
        if (IsKeyPressed(KEY_ESCAPE)) {
            result.type = MenuResult::Quit;
            return result;
        }
        result.type = MenuResult::None;
        return result;
    }

    if (m_screen == Screen::NewGame) {
        int yPos = 170;
        int fieldW = 420;
        int fieldH = 38;
        int fieldX = (constants::SCREEN_WIDTH - fieldW) / 2;

        Rectangle nameRect = {static_cast<float>(fieldX), static_cast<float>(yPos),
                              static_cast<float>(fieldW), static_cast<float>(fieldH)};

        yPos += 70;

        const char* sizes[] = {"Small", "Medium", "Large"};
        Rectangle sizeRects[3];
        int sizeStartX = fieldX;
        for (int i = 0; i < 3; ++i) {
            int sw = 126;
            sizeRects[i] = {
                static_cast<float>(sizeStartX + i * (sw + 16)),
                static_cast<float>(yPos - 4),
                static_cast<float>(sw),
                static_cast<float>(fieldH + 4)
            };
        }

        yPos += 70;

        int createW = 260;
        int createH = 44;
        Rectangle createRect = {
            (constants::SCREEN_WIDTH - createW) / 2.0f, static_cast<float>(yPos),
            static_cast<float>(createW), static_cast<float>(createH)
        };

        yPos += 70;

        int backW = 120;
        int backH = 36;
        Rectangle backRect = {
            (constants::SCREEN_WIDTH - backW) / 2.0f, static_cast<float>(yPos),
            static_cast<float>(backW), static_cast<float>(backH)
        };

        
        if (isClicked(nameRect)) {
            m_selectedOption = 0;
            m_editingName = true;
        }
        for (int i = 0; i < 3; ++i) {
            if (isClicked(sizeRects[i])) {
                m_selectedOption = 1;
                m_worldSizeIndex = i;
                m_editingName = false;
            }
        }
        if (isClicked(createRect)) {
            m_selectedOption = 2;
            m_editingName = false;
            if (m_worldName.empty()) m_worldName = "World";
            int freeSlot = -1;
            refreshSlots();
            for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
                if (!m_slots[i].occupied) { freeSlot = i; break; }
            }
            if (freeSlot < 0) freeSlot = 0;
            result.type = MenuResult::StartNewGame;
            result.slot = freeSlot;
            result.worldName = m_worldName;
            switch (m_worldSizeIndex) {
                case 0: result.worldSize = WorldSize::Small; break;
                case 1: result.worldSize = WorldSize::Medium; break;
                case 2: result.worldSize = WorldSize::Large; break;
            }
            reset();
            return result;
        }
        if (isClicked(backRect)) {
            m_editingName = false;
            m_screen = Screen::Main;
            m_selectedOption = 0;
        }

        
        if (mouseMoved) {
            if (isHovered(nameRect)) m_selectedOption = 0;
            else if (isHovered(sizeRects[0]) || isHovered(sizeRects[1]) || isHovered(sizeRects[2])) m_selectedOption = 1;
            else if (isHovered(createRect)) m_selectedOption = 2;
            else if (isHovered(backRect)) m_selectedOption = 3;
        }

        
        if (m_editingName) {
            int key = GetCharPressed();
            while (key > 0) {
                if (key >= 32 && key <= 126 && m_worldName.size() < 24)
                    m_worldName += static_cast<char>(key);
                key = GetCharPressed();
            }
            if (IsKeyPressed(KEY_BACKSPACE) && !m_worldName.empty())
                m_worldName.pop_back();
            if (IsKeyPressed(KEY_ENTER))
                m_editingName = false;
        }

        

        
        if (IsKeyPressed(KEY_ESCAPE)) {
            if (m_editingName) {
                m_editingName = false;
            } else {
                m_screen = Screen::Main;
                m_selectedOption = 0;
            }
        }

        
        

        result.type = MenuResult::None;
        return result;
    }

    if (m_screen == Screen::LoadGame) {
        int slotH = 60;
        int slotW = 880;
        int slotStartX = (constants::SCREEN_WIDTH - slotW) / 2;
        int slotStartY = 150;

        Rectangle slotRects[SaveManager::SLOT_COUNT];
        for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
            slotRects[i] = {
                static_cast<float>(slotStartX),
                static_cast<float>(slotStartY + i * (slotH + 10)),
                static_cast<float>(slotW),
                static_cast<float>(slotH)
            };
        }

        int backW = 120;
        int backH = 36;
        int backY = slotStartY + SaveManager::SLOT_COUNT * (slotH + 10) + 20;
        Rectangle backRect = {
            (constants::SCREEN_WIDTH - backW) / 2.0f,
            static_cast<float>(backY),
            static_cast<float>(backW),
            static_cast<float>(backH)
        };

        
        for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
            if (isClicked(slotRects[i])) {
                if (m_slots[i].occupied) {
                    result.type = MenuResult::LoadSlot;
                    result.slot = i;
                    reset();
                    return result;
                }
            }
        }
        if (isClicked(backRect)) {
            m_screen = Screen::Main;
            m_selectedOption = 1;
        }

        
        if (mouseMoved) {
            bool hovered = false;
            for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
                if (isHovered(slotRects[i])) {
                    m_selectedOption = i;
                    hovered = true;
                    break;
                }
            }
            if (!hovered && isHovered(backRect))
                m_selectedOption = SaveManager::SLOT_COUNT;
        }
        if (IsKeyPressed(KEY_DELETE) &&
            m_selectedOption >= 0 && m_selectedOption < SaveManager::SLOT_COUNT &&
            m_slots[m_selectedOption].occupied) {
            SaveManager::deleteSlot(m_selectedOption);
            refreshSlots();
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            m_screen = Screen::Main;
            m_selectedOption = 1;
        }

        result.type = MenuResult::None;
        return result;
    }

    result.type = MenuResult::None;
    return result;
}

void MainMenu::render() const {
    DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{20, 20, 30, 255});

    if (m_screen == Screen::Main) {
        const char* title = "Terraria";
        int titleSize = 60;
        int titleWidth = MeasureText(title, titleSize);
        DrawText(title, (constants::SCREEN_WIDTH - titleWidth) / 2, 100, titleSize, GREEN);

        const char* options[] = {"Create World", "Load World", "Settings", "Quit"};
        int optionSize = 30;

        for (int i = 0; i < m_mainOptionCount; ++i) {
            int optWidth = MeasureText(options[i], optionSize);
            int yPos = 260 + i * 60;
            Color color = (i == m_selectedOption) ? WHITE : GRAY;

            int totalW = optWidth + 48;
            int rectX = (constants::SCREEN_WIDTH - totalW) / 2;
            if (i == m_selectedOption) {
                DrawRectangle(rectX - 4, yPos - 4, totalW + 8, optionSize + 12,
                              Color{40, 40, 55, 255});
                DrawRectangleLines(rectX - 4, yPos - 4, totalW + 8, optionSize + 12,
                                   Color{80, 80, 120, 255});
            }
            DrawText(options[i], (constants::SCREEN_WIDTH - optWidth) / 2, yPos, optionSize, color);
            if (i == m_selectedOption) {
                DrawText("> ", (constants::SCREEN_WIDTH - optWidth) / 2 - 40, yPos, optionSize, GREEN);
            }
        }
        return;
    }

    if (m_screen == Screen::NewGame) {
        const char* title = "Create World";
        int titleSize = 50;
        int titleWidth = MeasureText(title, titleSize);
        DrawText(title, (constants::SCREEN_WIDTH - titleWidth) / 2, 60, titleSize, GREEN);

        int yPos = 170;
        int fieldW = 420;
        int fieldH = 38;
        int fieldX = (constants::SCREEN_WIDTH - fieldW) / 2;

        DrawText("World Name", fieldX, yPos - 24, 18, Color{150, 150, 160, 255});

        Color nameBorderColor;
        if (m_selectedOption == 0 && m_editingName) {
            nameBorderColor = YELLOW;
        } else if (m_selectedOption == 0) {
            nameBorderColor = WHITE;
        } else {
            nameBorderColor = Color{80, 80, 100, 255};
        }
        DrawRectangle(fieldX, yPos, fieldW, fieldH, Color{35, 35, 45, 255});
        DrawRectangleLines(fieldX, yPos, fieldW, fieldH, nameBorderColor);

        Color textColor = (m_worldName.empty() && !m_editingName) ? Color{100, 100, 100, 255} : WHITE;
        std::string displayName = m_worldName.empty() && !m_editingName
            ? "Enter world name..." : (m_worldName + (m_editingName ? "_" : ""));
        DrawText(displayName.c_str(), fieldX + 8, yPos + 7, 22, textColor);

        if (m_selectedOption == 0 && m_editingName) {
            DrawText("[Enter to confirm]", fieldX + fieldW + 12, yPos + 10, 16, DARKGRAY);
        }

        yPos += 70;

        DrawText("World Size", fieldX, yPos - 24, 18, Color{150, 150, 160, 255});

        const char* sizes[] = {"Small", "Medium", "Large"};
        const char* sizeDescs[] = {"1024x256", "2048x400", "4096x600"};
        int sizeStartX = fieldX;

        for (int i = 0; i < 3; ++i) {
            int sw = 126;
            int sh = fieldH + 4;
            int sx = sizeStartX + i * (sw + 16);
            bool selected = (i == m_worldSizeIndex);
            bool hovered = (m_selectedOption == 1 && selected);

            Color bg = selected ? Color{50, 50, 70, 255} : Color{30, 30, 40, 255};
            Color border = hovered ? WHITE : (selected ? GREEN : Color{60, 60, 80, 255});
            Color textC = selected ? WHITE : Color{120, 120, 140, 255};

            DrawRectangle(sx, yPos, sw, sh, bg);
            DrawRectangleLines(sx, yPos, sw, sh, border);
            DrawText(sizes[i], sx + (sw - MeasureText(sizes[i], 20)) / 2, yPos + 4, 20, textC);
            DrawText(sizeDescs[i], sx + (sw - MeasureText(sizeDescs[i], 14)) / 2,
                     yPos + 26, 14, Color{80, 80, 100, 255});
        }

        yPos += 70;

        
        Color createBg = (m_selectedOption == 2) ? Color{50, 70, 50, 255} : Color{35, 45, 35, 255};
        Color createBorder = (m_selectedOption == 2) ? GREEN : Color{60, 80, 60, 255};
        int createW = 260;
        int createH = 44;
        int createX = (constants::SCREEN_WIDTH - createW) / 2;
        DrawRectangle(createX, yPos, createW, createH, createBg);
        DrawRectangleLines(createX, yPos, createW, createH, createBorder);
        const char* createText = "Create World";
        DrawText(">", createX - 30, yPos + 10, 24, GREEN);
        DrawText(createText, createX + (createW - MeasureText(createText, 24)) / 2,
                 yPos + 10, 24, (m_selectedOption == 2) ? WHITE : GRAY);

        yPos += 70;

        
        Color backBg = (m_selectedOption == 3) ? Color{50, 45, 45, 255} : Color{35, 35, 35, 255};
        Color backBorder = (m_selectedOption == 3) ? Color{180, 100, 100, 255} : Color{60, 60, 60, 255};
        int backW = 120;
        int backH = 36;
        int backX = (constants::SCREEN_WIDTH - backW) / 2;
        DrawRectangle(backX, yPos, backW, backH, backBg);
        DrawRectangleLines(backX, yPos, backW, backH, backBorder);
        const char* backText = "Back";
        DrawText(backText, backX + (backW - MeasureText(backText, 22)) / 2,
                 yPos + 7, 22, (m_selectedOption == 3) ? WHITE : GRAY);

        return;
    }

    if (m_screen == Screen::LoadGame) {
        const char* title = "Load World";
        int titleSize = 50;
        int titleWidth = MeasureText(title, titleSize);
        DrawText(title, (constants::SCREEN_WIDTH - titleWidth) / 2, 60, titleSize, GREEN);

        const char* slotLabels[] = {"Slot 1", "Slot 2", "Slot 3", "Slot 4", "Slot 5"};
        int slotH = 60;
        int slotW = 880;
        int slotStartX = (constants::SCREEN_WIDTH - slotW) / 2;
        int slotStartY = 150;

        for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
            int yPos = slotStartY + i * (slotH + 10);
            bool hovered = (i == m_selectedOption);

            Color bgColor = hovered ? Color{40, 40, 55, 255} : Color{30, 30, 40, 255};
            Color borderColor = hovered ? Color{80, 80, 120, 255} : Color{45, 45, 55, 255};

            DrawRectangle(slotStartX, yPos, slotW, slotH, bgColor);
            DrawRectangleLines(slotStartX, yPos, slotW, slotH, borderColor);

            DrawText(slotLabels[i], slotStartX + 20, yPos + 8, 22,
                     hovered ? WHITE : Color{150, 150, 160, 255});

            if (m_slots[i].occupied) {
                DrawText(m_slots[i].name.c_str(), slotStartX + 140, yPos + 8, 22, WHITE);
                const char* sz = worldSizeName(m_slots[i].size);
                DrawText(sz, slotStartX + 140, yPos + 32, 16, DARKGRAY);
                if (hovered) {
                    DrawText("DEL to delete", slotStartX + slotW - 130, yPos + 18, 16,
                             Color{180, 80, 80, 255});
                }
            } else {
                DrawText("Empty", slotStartX + 140, yPos + 18, 20, Color{80, 80, 80, 255});
            }
        }

        int backY = slotStartY + SaveManager::SLOT_COUNT * (slotH + 10) + 20;
        bool backHovered = (m_selectedOption == SaveManager::SLOT_COUNT);
        Color backBg = backHovered ? Color{50, 45, 45, 255} : Color{35, 35, 35, 255};
        Color backBorder = backHovered ? Color{180, 100, 100, 255} : Color{60, 60, 60, 255};
        int backW = 120;
        int backH = 36;
        int backX = (constants::SCREEN_WIDTH - backW) / 2;
        DrawRectangle(backX, backY, backW, backH, backBg);
        DrawRectangleLines(backX, backY, backW, backH, backBorder);
        const char* backText = "Back";
        DrawText(backText, backX + (backW - MeasureText(backText, 22)) / 2,
                 backY + 7, 22, backHovered ? WHITE : GRAY);
    }
}
