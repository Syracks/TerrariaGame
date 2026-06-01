#pragma once

#include <raylib.h>
#include <string>
#include <array>
#include "core/Constants.hpp"
#include "save/SaveManager.hpp"

struct MenuResult {
    enum Type { None, StartNewGame, LoadSlot, Settings, Quit };
    Type type = None;
    int slot = -1;
    std::string worldName;
    WorldSize worldSize = WorldSize::Medium;
    Difficulty difficulty = Difficulty::Normal;
};

class MainMenu {
public:
    MainMenu();
    MenuResult update();
    void render() const;
    bool isVisible() const { return m_visible; }
    void setVisible(bool v);
    void reset();

private:
    enum class Screen { Main, NewGame, LoadGame };

    Screen m_screen = Screen::Main;
    bool m_visible = true;
    int m_selectedOption = 0;

    int m_mainOptionCount = 4;
    int m_newGameOptionCount = 5; 
    int m_loadGameOptionCount = 6; 

    std::string m_worldName;
    bool m_editingName = false;
    int m_worldSizeIndex = 1;
    int m_difficultyIndex = 0;
    int m_selectedSlot = -1;

    std::array<SlotInfo, SaveManager::SLOT_COUNT> m_slots{};

    void refreshSlots();

    Vector2 m_lastMousePos{-100, -100};
    int m_confirmDeleteSlot = -1;
    int m_confirmOverwriteSlot = -1;

    static const char* worldSizeText(int index);
    static const char* difficultyText(int index);
};
