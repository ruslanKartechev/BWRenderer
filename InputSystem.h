#pragma once
#include <cglm/cglm.h>
#include "MyTypes.h"

constexpr int KEYS_BUFFER_SIZE = 256;

enum class GameInputKey{
    KEY_UNKNOWN = 0,

    // Alpha-numeric
    KEY_A, KEY_B, KEY_C, KEY_D, KEY_E, KEY_F, KEY_G, KEY_H, KEY_I, KEY_J,
    KEY_K, KEY_L, KEY_M, KEY_N, KEY_O, KEY_P, KEY_Q, KEY_R, KEY_S, KEY_T,
    KEY_U, KEY_V, KEY_W, KEY_X, KEY_Y, KEY_Z,
    KEY_0, KEY_1, KEY_2, KEY_3, KEY_4, KEY_5, KEY_6, KEY_7, KEY_8, KEY_9,

    // Controls
    KEY_ESCAPE,
    KEY_SPACE,
    KEY_ENTER,
    KEY_TAB,
    KEY_BACKSPACE,
    KEY_LSHIFT,
    KEY_RSHIFT,
    KEY_LCTRL,
    KEY_RCTRL,
    KEY_LALT,
    KEY_RALT,

    // Arrow Keys
    KEY_LEFT, KEY_UP, KEY_RIGHT, KEY_DOWN,

    // Mouse Buttons
    MOUSE_BUTTON_LEFT,
    MOUSE_BUTTON_RIGHT,
    MOUSE_BUTTON_MIDDLE,

    KEY_MAX_KEYS // Used for sizing arrays
};


// Clean, platform-agnostic polling API
bool Input_IsKeyDown(GameInputKey engineKey);
bool Input_IsKeyHeld(GameInputKey engineKey);
bool Input_IsKeyUp(GameInputKey engineKey);

bool Input_IsMouseButtonDown(GameInputKey mouseButton);
bool Input_IsMouseButtonHeld(GameInputKey mouseButton);
bool Input_IsMouseButtonUp(GameInputKey mouseButton);

void Input_GetMousePosition(vec2 dest);
void Input_GetMouseDelta(vec2 dest);

// i32ernal hooks for your engine pipeline
void Input_Initialize();
void Input_Update();

// The new abstraction bridge functions called by your Window abstraction layer
void Input_SetKeyState(i32 engineKey, bool isDown);
void Input_SetMousePosition(float x, float y);
void Input_SetAbsolutePosition(float x, float y);