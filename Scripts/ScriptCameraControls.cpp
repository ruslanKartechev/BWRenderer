#include "ScriptCameraControls.h"

#include "../Engine.h"
#include "../ProjectSettings.h"
#include "../InputSystem.h"
#include "../GameTime.h"
#include <format>
#include <string>
#include <iostream>
#include "imgui.h"



class ProjectSettings;

void ScriptCameraControls::ControlCamera(f32 dt) {
    auto& engine = *Engine::GetInstance();
    Transform& cameraTransform = engine.scene.transforms.GetItemRef(engine.scene.camera.transformHandle);
    float moveSpeed = engine.settings.Camera_Move_Speed;
    float rotSpeed  = engine.settings.Camera_Rotation_Speed;

    vec3 localMove = {};
    float verticalShift = 0;
    if (Input_IsKeyHeld(GameInputKey::KEY_W)) {
        localMove[2] = 1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_A)) {
        localMove[0] = -1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_S)) {
        localMove[2] = -1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_D)) {
        localMove[0] = 1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_E)) {
        verticalShift = 1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_Q)) {
        verticalShift = -1;
    }
    vec2 mousePosition;
    Input_GetMousePosition(mousePosition);
    bool isMoving = localMove[0] != 0 || localMove[1] != 0 || localMove[2] != 0;
    if (isMoving) {
        elapsedTimeMoving += Time_Dt();
    }
    else{
        elapsedTimeMoving = 0.0;
    }

    if (Input_IsMouseButtonHeld(GameInputKey::MOUSE_BUTTON_RIGHT)) {
        vec2 mouseDelta;
        Input_GetMouseDelta(mouseDelta);
        glm_vec2_scale(mouseDelta, dt * rotSpeed, mouseDelta);

        vec3 eulersBefore;
        vec3 eulersAfter;
        Transform_QuatToEuler(cameraTransform.rotation, eulersBefore);

        Transform_RotateWorldY(cameraTransform, mouseDelta[0]);
        Transform_RotateLocalX(cameraTransform, -mouseDelta[1]);
        Transform_QuatToEuler(cameraTransform.rotation, eulersAfter);
    }

    vec3 worldMove;
    Transform_ToWorldVector(cameraTransform, localMove, worldMove);
    glm_vec3_scale(worldMove, dt * moveSpeed * (1 + elapsedTimeMoving), worldMove);

    vec3 verticalMove = {0,1,0};
    glm_vec3_scale(verticalMove, dt * moveSpeed * (1 + elapsedTimeMoving) * verticalShift, verticalMove);
    glm_vec3_add(worldMove, verticalMove, worldMove);
    glm_vec3_add(cameraTransform.position, worldMove, cameraTransform.position);
}

void ScriptCameraControls::ControlSettings() {
    auto& engine = *Engine::GetInstance();
    ProjectSettings& settings = engine.settings;

    if (Input_IsKeyDown(GameInputKey::KEY_1)) {
        settings.UseGammaCorrection  = !settings.UseGammaCorrection;
        std::cout << "[GammaCorrection]: " << settings.UseGammaCorrection << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_P)) {
        settings.DevRenderDepths  = !settings.DevRenderDepths;
        std::cout << "[Dev Depths Only]: " << settings.DevRenderDepths << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_N)) {
        settings.DevRenderNormals  = !settings.DevRenderNormals;
        std::cout << "[Dev Normals Only]: " << settings.DevRenderNormals << std::endl;
    }

    if (Input_IsKeyDown(GameInputKey::KEY_J)) {
        settings.RenderSkyBox  = !settings.RenderSkyBox;
        std::cout << "[Render Skybox]: " << settings.RenderSkyBox << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_K)) {
        settings.RenderShadows  = !settings.RenderShadows;
        std::cout << "[Render Shadows]: " << settings.RenderShadows << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_L)) {
        settings.PostProcess  = !settings.PostProcess;
        std::cout << "[Render PostProcess]: " << settings.PostProcess << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_B)) {
        settings.PostProcess_Bloom  = !settings.PostProcess_Bloom;
        std::cout << "[Render PostProcess_Bloom]: " << settings.PostProcess_Bloom << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_R)) {
        settings.PostProcess_SSR  = !settings.PostProcess_SSR;
        std::cout << "[Render PostProcess_SSR]: " << settings.PostProcess_SSR << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_O)) {
        settings.PostProcess_DOF  = !settings.PostProcess_DOF;
        std::cout << "[Render PostProcess_DOF]: " << settings.PostProcess_DOF << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_P)) {
        settings.PostProcess_ToneMapping  = !settings.PostProcess_ToneMapping;
        std::cout << "[Render PostProcess_DOF]: " << settings.PostProcess_ToneMapping << std::endl;
    }
}


void ScriptCameraControls::Create() {
}

void ScriptCameraControls::Start() {
    auto& engine = *Engine::GetInstance();
    engine.scene.camera.UpdateFarPlane(1500);
}

void ScriptCameraControls::GamePause() {
}
void ScriptCameraControls::GameResume() {
}
void ScriptCameraControls::Quit() {
}


void ScriptCameraControls::Update(f32 deltaTime) {
    ControlCamera(deltaTime);
    ControlSettings();
}

void ScriptCameraControls::PhysicsUpdate(f32 deltaTime) {
}


void ScriptCameraControls::GUIUpdate(f32 deltaTime) {
    auto& engine = *Engine::GetInstance();
    Transform& cameraTransform = engine.scene.transforms.GetItemRef(engine.scene.camera.transformHandle);

    ImGui::Begin("Camera Controls");
    ImGui::SliderFloat("Move Speed", &engine.settings.Camera_Move_Speed, 1.0f, 100.0f);
    ImGui::SliderFloat("Rot. Speed", &engine.settings.Camera_Rotation_Speed, 1.0f, 50.0f);

    std::string posStr = std::format("Position: {0:.2},{1:.2},{2:.2}", cameraTransform.position[0], cameraTransform.position[1], cameraTransform.position[2]);
    ImGui::LabelText("##", posStr.c_str());

    ImGui::End();
}