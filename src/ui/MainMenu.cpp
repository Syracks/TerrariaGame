#include "MainMenu.hpp"
#include "core/Constants.hpp"
#include "core/Math.hpp"
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
        m_worldSizeIndex = 1;
        m_difficultyIndex = 0;
        refreshSlots();
    }
}

void MainMenu::reset() {
    m_screen = Screen::Main;
    m_selectedOption = 0;
    m_worldName.clear();
    m_editingName = false;
    m_worldSizeIndex = 1;
    m_difficultyIndex = 0;
    m_selectedSlot = -1;
    m_confirmDeleteSlot = -1;
    m_confirmOverwriteSlot = -1;
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

const char* MainMenu::difficultyText(int index) {
    switch (index) {
        case 0: return "Normal";
        case 1: return "Hardcore";
        default: return "Normal";
    }
}

static bool isClicked(Rectangle rect) {
    if (!IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) return false;
    Vector2 mp = math::getVirtualMouse();
    return CheckCollisionPointRec(mp, rect);
}

static bool isHovered(Rectangle rect) {
    Vector2 mp = math::getVirtualMouse();
    return CheckCollisionPointRec(mp, rect);
}

MenuResult MainMenu::update() {
    MenuResult result;

    if (!m_visible) {
        result.type = MenuResult::None;
        return result;
    }

    Vector2 mousePos = math::getVirtualMouse();
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

        yPos += 60;

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

        yPos += 75;

        Rectangle diffRects[2];
        int diffStartX = fieldX;
        for (int i = 0; i < 2; ++i) {
            int sw = 140;
            diffRects[i] = {
                static_cast<float>(diffStartX + i * (sw + 16)),
                static_cast<float>(yPos - 4),
                static_cast<float>(sw),
                static_cast<float>(fieldH + 4)
            };
        }

        yPos += 60;

        int createW = 260;
        int createH = 44;
        Rectangle createRect = {
            (constants::SCREEN_WIDTH - createW) / 2.0f, static_cast<float>(yPos),
            static_cast<float>(createW), static_cast<float>(createH)
        };

        yPos += 60;

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
        for (int i = 0; i < 2; ++i) {
            if (isClicked(diffRects[i])) {
                m_selectedOption = 2;
                m_difficultyIndex = i;
                m_editingName = false;
            }
        }
        if (isClicked(createRect) && m_confirmOverwriteSlot < 0) {
            m_selectedOption = 3;
            m_editingName = false;
            if (m_worldName.empty()) m_worldName = "World";
            refreshSlots();
            int freeSlot = -1;
            for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
                if (!m_slots[i].occupied) { freeSlot = i; break; }
            }
            if (freeSlot >= 0) {
                result.type = MenuResult::StartNewGame;
                result.slot = freeSlot;
                result.worldName = m_worldName;
                switch (m_worldSizeIndex) {
                    case 0: result.worldSize = WorldSize::Small; break;
                    case 1: result.worldSize = WorldSize::Medium; break;
                    case 2: result.worldSize = WorldSize::Large; break;
                }
                result.difficulty = (m_difficultyIndex == 1) ? Difficulty::Hardcore : Difficulty::Normal;
                reset();
                return result;
            } else {
                m_confirmOverwriteSlot = 0;
            }
        }

        if (m_confirmOverwriteSlot >= 0) {
            int slot = m_confirmOverwriteSlot;
            int dialogW = 420;
            int dialogH = 180;
            int dialogX = (constants::SCREEN_WIDTH - dialogW) / 2;
            int dialogY = (constants::SCREEN_HEIGHT - dialogH) / 2;
            Rectangle yesRect = {static_cast<float>(dialogX + 60), static_cast<float>(dialogY + 120), 120.0f, 40.0f};
            Rectangle noRect = {static_cast<float>(dialogX + dialogW - 180), static_cast<float>(dialogY + 120), 120.0f, 40.0f};

            if (isClicked(yesRect)) {
                result.type = MenuResult::StartNewGame;
                result.slot = slot;
                result.worldName = m_worldName.empty() ? "World" : m_worldName;
                switch (m_worldSizeIndex) {
                    case 0: result.worldSize = WorldSize::Small; break;
                    case 1: result.worldSize = WorldSize::Medium; break;
                    case 2: result.worldSize = WorldSize::Large; break;
                }
                result.difficulty = (m_difficultyIndex == 1) ? Difficulty::Hardcore : Difficulty::Normal;
                m_confirmOverwriteSlot = -1;
                reset();
                return result;
            } else if (isClicked(noRect)) {
                m_confirmOverwriteSlot = -1;
            } else if (IsKeyPressed(KEY_ESCAPE)) {
                m_confirmOverwriteSlot = -1;
            }
            result.type = MenuResult::None;
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
            else if (isHovered(diffRects[0]) || isHovered(diffRects[1])) m_selectedOption = 2;
            else if (isHovered(createRect)) m_selectedOption = 3;
            else if (isHovered(backRect)) m_selectedOption = 4;
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
        Rectangle delRects[SaveManager::SLOT_COUNT];
        for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
            int yPos = slotStartY + i * (slotH + 10);
            slotRects[i] = {
                static_cast<float>(slotStartX),
                static_cast<float>(yPos),
                static_cast<float>(slotW),
                static_cast<float>(slotH)
            };
            delRects[i] = {
                static_cast<float>(slotStartX + slotW - 100),
                static_cast<float>(yPos + 12),
                80.0f,
                36.0f
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

        if (m_confirmDeleteSlot >= 0 && m_confirmDeleteSlot < SaveManager::SLOT_COUNT) {
            int dialogW = 360;
            int dialogH = 160;
            int dialogX = (constants::SCREEN_WIDTH - dialogW) / 2;
            int dialogY = (constants::SCREEN_HEIGHT - dialogH) / 2;
            Rectangle yesRect = {static_cast<float>(dialogX + 50), static_cast<float>(dialogY + 90), 100.0f, 40.0f};
            Rectangle noRect = {static_cast<float>(dialogX + dialogW - 150), static_cast<float>(dialogY + 90), 100.0f, 40.0f};

            if (isClicked(yesRect)) {
                SaveManager::deleteSlot(m_confirmDeleteSlot);
                m_confirmDeleteSlot = -1;
                refreshSlots();
            } else if (isClicked(noRect)) {
                m_confirmDeleteSlot = -1;
            } else if (IsKeyPressed(KEY_ESCAPE)) {
                m_confirmDeleteSlot = -1;
            }
        } else {
            for (int i = 0; i < SaveManager::SLOT_COUNT; ++i) {
                if (m_slots[i].occupied && isClicked(delRects[i])) {
                    m_confirmDeleteSlot = i;
                } else if (isClicked(slotRects[i])) {
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
        }

        if (IsKeyPressed(KEY_ESCAPE) && m_confirmDeleteSlot < 0) {
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

        yPos += 60;

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

        yPos += 75;

        DrawText("Difficulty", fieldX, yPos - 24, 18, Color{150, 150, 160, 255});

        const char* diffLabels[] = {"Normal", "Hardcore"};
        const char* diffDescs[] = {"Standard", "Permadeath"};
        int diffStartX = fieldX;

        for (int i = 0; i < 2; ++i) {
            int sw = 140;
            int sh = fieldH + 4;
            int sx = diffStartX + i * (sw + 16);
            bool selected = (i == m_difficultyIndex);
            bool hovered = (m_selectedOption == 2 && selected);

            Color bg = selected ? Color{50, 50, 70, 255} : Color{30, 30, 40, 255};
            Color border = hovered ? WHITE : (selected ? GREEN : Color{60, 60, 80, 255});
            Color textC = selected ? WHITE : Color{120, 120, 140, 255};

            DrawRectangle(sx, yPos, sw, sh, bg);
            DrawRectangleLines(sx, yPos, sw, sh, border);
            Color hardcoreTint = (i == 1 && selected) ? Color{255, 100, 100, 255} : textC;
            DrawText(diffLabels[i], sx + (sw - MeasureText(diffLabels[i], 20)) / 2, yPos + 4, 20, hardcoreTint);
            DrawText(diffDescs[i], sx + (sw - MeasureText(diffDescs[i], 14)) / 2,
                     yPos + 26, 14, Color{80, 80, 100, 255});
        }

        yPos += 60;

        
        Color createBg = (m_selectedOption == 3) ? Color{50, 70, 50, 255} : Color{35, 45, 35, 255};
        Color createBorder = (m_selectedOption == 3) ? GREEN : Color{60, 80, 60, 255};
        int createW = 260;
        int createH = 44;
        int createX = (constants::SCREEN_WIDTH - createW) / 2;
        DrawRectangle(createX, yPos, createW, createH, createBg);
        DrawRectangleLines(createX, yPos, createW, createH, createBorder);
        const char* createText = "Create World";
        DrawText(">", createX - 30, yPos + 10, 24, GREEN);
        DrawText(createText, createX + (createW - MeasureText(createText, 24)) / 2,
                 yPos + 10, 24, (m_selectedOption == 3) ? WHITE : GRAY);

        yPos += 60;

        
        Color backBg = (m_selectedOption == 4) ? Color{50, 45, 45, 255} : Color{35, 35, 35, 255};
        Color backBorder = (m_selectedOption == 4) ? Color{180, 100, 100, 255} : Color{60, 60, 60, 255};
        int backW = 120;
        int backH = 36;
        int backX = (constants::SCREEN_WIDTH - backW) / 2;
        DrawRectangle(backX, yPos, backW, backH, backBg);
        DrawRectangleLines(backX, yPos, backW, backH, backBorder);
        const char* backText = "Back";
        DrawText(backText, backX + (backW - MeasureText(backText, 22)) / 2,
                 yPos + 7, 22, (m_selectedOption == 4) ? WHITE : GRAY);

        if (m_confirmOverwriteSlot >= 0) {
            int slot = m_confirmOverwriteSlot;
            int dialogW = 420;
            int dialogH = 180;
            int dialogX = (constants::SCREEN_WIDTH - dialogW) / 2;
            int dialogY = (constants::SCREEN_HEIGHT - dialogH) / 2;

            DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{0, 0, 0, 160});
            DrawRectangle(dialogX, dialogY, dialogW, dialogH, Color{30, 30, 40, 255});
            DrawRectangleLines(dialogX, dialogY, dialogW, dialogH, Color{180, 120, 40, 255});

            const char* msg = "Slot is occupied! Overwrite?";
            int msgW = MeasureText(msg, 22);
            DrawText(msg, dialogX + (dialogW - msgW) / 2, dialogY + 24, 22, WHITE);

            const SlotInfo& si = m_slots[slot];
            std::string info = si.name + " (" + worldSizeName(si.size) + " - " + difficultyName(si.difficulty) + ")";
            int infoW = MeasureText(info.c_str(), 18);
            DrawText(info.c_str(), dialogX + (dialogW - infoW) / 2, dialogY + 56, 18, Color{180, 180, 180, 255});

            std::string newInfo = "New: " + m_worldName + " (" + worldSizeText(m_worldSizeIndex) + " - " + difficultyText(m_difficultyIndex) + ")";
            int newInfoW = MeasureText(newInfo.c_str(), 16);
            DrawText(newInfo.c_str(), dialogX + (dialogW - newInfoW) / 2, dialogY + 82, 16, Color{140, 140, 160, 255});

            Rectangle yesRect = {static_cast<float>(dialogX + 60), static_cast<float>(dialogY + 120), 120.0f, 40.0f};
            Rectangle noRect = {static_cast<float>(dialogX + dialogW - 180), static_cast<float>(dialogY + 120), 120.0f, 40.0f};

            bool yesHov = CheckCollisionPointRec(math::getVirtualMouse(), yesRect);
            bool noHov = CheckCollisionPointRec(math::getVirtualMouse(), noRect);
            DrawRectangleRec(yesRect, yesHov ? Color{180, 80, 40, 255} : Color{140, 60, 30, 255});
            DrawRectangleLinesEx(yesRect, 1, yesHov ? Color{255, 150, 50, 255} : Color{200, 100, 40, 255});
            DrawText("Overwrite", yesRect.x + (120 - MeasureText("Overwrite", 20)) / 2, yesRect.y + 10, 20, WHITE);
            DrawRectangleRec(noRect, noHov ? Color{50, 50, 60, 255} : Color{35, 35, 45, 255});
            DrawRectangleLinesEx(noRect, 1, noHov ? WHITE : Color{80, 80, 100, 255});
            DrawText("Cancel", noRect.x + (120 - MeasureText("Cancel", 20)) / 2, noRect.y + 10, 20, WHITE);
        }

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

                std::string infoLine = std::string(worldSizeName(m_slots[i].size)) + " - " + difficultyName(m_slots[i].difficulty);
                DrawText(infoLine.c_str(), slotStartX + 140, yPos + 32, 16, DARKGRAY);

                Rectangle delRect = {
                    static_cast<float>(slotStartX + slotW - 100),
                    static_cast<float>(yPos + 12), 80.0f, 36.0f
                };
                bool delHovered = CheckCollisionPointRec(math::getVirtualMouse(), delRect);
                DrawRectangleRec(delRect, delHovered ? Color{180, 40, 40, 255} : Color{120, 30, 30, 255});
                DrawRectangleLinesEx(delRect, 1, delHovered ? Color{255, 80, 80, 255} : Color{180, 50, 50, 255});
                const char* delText = "Delete";
                int delTW = MeasureText(delText, 18);
                DrawText(delText, slotStartX + slotW - 100 + (80 - delTW) / 2, yPos + 18, 18, WHITE);
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

        if (m_confirmDeleteSlot >= 0 && m_confirmDeleteSlot < SaveManager::SLOT_COUNT) {
            DrawRectangle(0, 0, constants::SCREEN_WIDTH, constants::SCREEN_HEIGHT, Color{0, 0, 0, 160});

            int dialogW = 360;
            int dialogH = 160;
            int dialogX = (constants::SCREEN_WIDTH - dialogW) / 2;
            int dialogY = (constants::SCREEN_HEIGHT - dialogH) / 2;
            DrawRectangle(dialogX, dialogY, dialogW, dialogH, Color{30, 30, 40, 255});
            DrawRectangleLines(dialogX, dialogY, dialogW, dialogH, Color{180, 80, 80, 255});

            const char* msg = "Delete this world?";
            int msgW = MeasureText(msg, 24);
            DrawText(msg, dialogX + (dialogW - msgW) / 2, dialogY + 30, 24, WHITE);

            const SlotInfo& si = m_slots[m_confirmDeleteSlot];
            std::string info = si.name + " (" + worldSizeName(si.size) + " - " + difficultyName(si.difficulty) + ")";
            int infoW = MeasureText(info.c_str(), 18);
            DrawText(info.c_str(), dialogX + (dialogW - infoW) / 2, dialogY + 60, 18, Color{180, 180, 180, 255});

            Rectangle yesRect = {static_cast<float>(dialogX + 50), static_cast<float>(dialogY + 95), 100.0f, 40.0f};
            Rectangle noRect = {static_cast<float>(dialogX + dialogW - 150), static_cast<float>(dialogY + 95), 100.0f, 40.0f};

            bool yesHov = CheckCollisionPointRec(math::getVirtualMouse(), yesRect);
            bool noHov = CheckCollisionPointRec(math::getVirtualMouse(), noRect);
            DrawRectangleRec(yesRect, yesHov ? Color{160, 40, 40, 255} : Color{120, 30, 30, 255});
            DrawRectangleLinesEx(yesRect, 1, yesHov ? Color{255, 80, 80, 255} : Color{180, 50, 50, 255});
            DrawText("Yes", yesRect.x + (100 - MeasureText("Yes", 22)) / 2, yesRect.y + 9, 22, WHITE);
            DrawRectangleRec(noRect, noHov ? Color{50, 50, 60, 255} : Color{35, 35, 45, 255});
            DrawRectangleLinesEx(noRect, 1, noHov ? WHITE : Color{80, 80, 100, 255});
            DrawText("No", noRect.x + (100 - MeasureText("No", 22)) / 2, noRect.y + 9, 22, WHITE);
        }
    }
}
