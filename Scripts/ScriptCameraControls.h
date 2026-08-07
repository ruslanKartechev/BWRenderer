#pragma once
#include "../MyTypes.h"

class ScriptCameraControls {
public:
    void Create();
    void Start();
    void GamePause();
    void GameResume();
    void Quit();

    void Update(f32 deltaTime);
    void PhysicsUpdate(f32 deltaTime);
    void GUIUpdate(f32 deltaTime);

private:
    f32 elapsedTimeMoving = 0.0f;
    void ControlSettings();
    void ControlCamera(f32);
};
