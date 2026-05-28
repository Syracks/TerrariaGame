#pragma once

#include <raylib.h>

namespace input {

inline bool isLeftPressed() {
    return IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT);
}

inline bool isRightPressed() {
    return IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT);
}

inline bool isJumpPressed() {
    return IsKeyPressed(KEY_SPACE) || IsKeyPressed(KEY_W) || IsKeyPressed(KEY_UP);
}

inline bool isJumpHeld() {
    return IsKeyDown(KEY_SPACE);
}

inline bool isMinePressed() {
    return IsMouseButtonPressed(MOUSE_BUTTON_LEFT);
}

inline bool isPlacePressed() {
    return IsMouseButtonPressed(MOUSE_BUTTON_RIGHT);
}

inline bool isSavePressed() {
    return IsKeyPressed(KEY_F5);
}

inline bool isLoadPressed() {
    return IsKeyPressed(KEY_F9);
}

inline bool isPausePressed() {
    return IsKeyPressed(KEY_ESCAPE);
}

inline bool isMinimapToggled() {
    return IsKeyPressed(KEY_M);
}

inline int getHotbarSelection() {
    for (int i = 0; i < 8; ++i) {
        if (IsKeyPressed(static_cast<int>(KEY_ONE) + i))
            return i;
    }
    return -1;
}

}
