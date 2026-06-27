#include "InputSystem.h"
#include <string.h>
#define K2Int(v) static_cast<int>(v)

static bool s_currentKeys[KEYS_BUFFER_SIZE];
static bool s_previousKeys[KEYS_BUFFER_SIZE];

static vec2 s_currentAbsPosition = { 0.0f, 0.0f };
static vec2 s_currentMousePos = { 0.0f, 0.0f };
static vec2 s_previousMousePos = { 0.0f, 0.0f };
static vec2 s_mouseDelta = { 0.0f, 0.0f };

void Input_Initialize() {
    memset(s_currentKeys, 0, sizeof(s_currentKeys));
    memset(s_previousKeys, 0, sizeof(s_previousKeys));
}

void Input_Update() {
    memcpy(s_previousKeys, s_currentKeys, sizeof(s_currentKeys));
    glm_vec2_sub(s_currentMousePos, s_previousMousePos, s_mouseDelta);
    glm_vec2_copy(s_currentMousePos, s_previousMousePos);
}

bool Input_IsKeyDown(GameInputKey key) {
    return s_currentKeys[K2Int(key)] && !s_previousKeys[K2Int(key)];
}
bool Input_IsKeyHeld(GameInputKey key) {
    return s_currentKeys[K2Int(key)];
}
bool Input_IsKeyUp(GameInputKey key) {
    return !s_currentKeys[K2Int(key)] && s_previousKeys[K2Int(key)];
}

bool Input_IsMouseButtonDown(GameInputKey btn) {
    return s_currentKeys[K2Int(btn)];
}
bool Input_IsMouseButtonPressed(GameInputKey btn) {
    return s_currentKeys[K2Int(btn)] && !s_previousKeys[K2Int(btn)];
}
bool Input_IsMouseButtonUp(GameInputKey btn) {
    return !s_currentKeys[K2Int(btn)] && s_previousKeys[K2Int(btn)];
}

void Input_GetMousePosition(vec2 dest) {
    glm_vec2_copy(s_currentMousePos, dest);
}

void Input_GetMouseDelta(vec2 dest) {
    glm_vec2_copy(s_mouseDelta, dest);
}

// Abstraction Layer Targets
void Input_SetKeyState(i32 engineKey, bool isDown) {
    if (engineKey > static_cast<i32>(GameInputKey::KEY_UNKNOWN)
        && engineKey < static_cast<i32>(GameInputKey::KEY_MAX_KEYS))
    {
        s_currentKeys[engineKey] = isDown;
    }
}

void Input_SetMousePosition(f32 x, f32 y) {
    s_currentMousePos[0] = x;
    s_currentMousePos[1] = y;
}

void Input_SetAbsolutePosition(f32 x, f32 y) {
    s_currentAbsPosition[0] = x;
    s_currentAbsPosition[1] = y;
}