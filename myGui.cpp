#include <iostream>
#include "myGui.h"

#include "GameTime.h"
#include "imgui/imgui.h"
#include "imgui/imgui_impl_opengl3.h"
#include "GraphicsGL.h"
#include "InputSystem.h"
#include "ProgramWindow.h"

static bool demoOpen = false;

void MyGui::InitForWindow(ProgramWindow* window) {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    windowPtr = window;

    ImGuiIO& io = ImGui::GetIO();
    io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;
    io.BackendPlatformName = "Custom_Engine_Backend";
    io.ConfigDpiScaleFonts = true;
    io.DisplaySize = ImVec2(windowPtr->width, windowPtr->height);
    ImGui::StyleColorsDark();

    bool result = ImGui_ImplOpenGL3_Init("#version 450 core");
    if (result) {
        initialized = true;
        std::cout << "INITIALIZED IM GUI CORRECT" << std::endl;
    }
    else {
        std::cerr << "FAILED TO INIT IMGUI" << std::endl;

    }
}


void MyGui::EndFrame() {
    if (!initialized) {
        return;
    }
    // std::cout << "GUI --- EndFrame" << std::endl;
    ImGui::EndFrame();
}


void MyGui::StartFrame() {

    if (!initialized) {
        return;
    }
    // std::cout << "GUI --- StartFrame" << std::endl;
    ImGuiIO& io = ImGui::GetIO();
    io.DeltaTime = static_cast<f32>(Time_Dt());
    io.DisplaySize = ImVec2(static_cast<f32>(windowPtr->width), static_cast<f32>(windowPtr->height));

    ImGui_ImplOpenGL3_NewFrame();
    ImGui::NewFrame();
}



void MyGui::UpdateInputs() {
    ImGuiIO& io = ImGui::GetIO();
    {
        vec2 mousePos;
        Input_GetMousePosition(mousePos);
        f32 yPos = (f32)(windowPtr->height) - mousePos[1];
        io.AddMousePosEvent(mousePos[0], yPos);
    }
    auto MapKey = [&](GameInputKey myKey, ImGuiKey imguiKey) {
        io.AddKeyEvent(imguiKey, Input_IsKeyDown(myKey));
    };

    bool mouseLeft = Input_IsMouseButtonHeld(GameInputKey::MOUSE_BUTTON_LEFT);
    bool mouseMid = Input_IsMouseButtonHeld(GameInputKey::MOUSE_BUTTON_MIDDLE);
    bool mouseRight = Input_IsMouseButtonHeld(GameInputKey::MOUSE_BUTTON_RIGHT);
    io.AddMouseButtonEvent(0, mouseLeft);
    io.AddMouseButtonEvent(2, mouseMid);
    io.AddMouseButtonEvent(1, mouseRight);

    MapKey(GameInputKey::KEY_LEFT,  ImGuiKey_LeftArrow);
    MapKey(GameInputKey::KEY_RIGHT, ImGuiKey_RightArrow);
    MapKey(GameInputKey::KEY_UP,    ImGuiKey_UpArrow);
    MapKey(GameInputKey::KEY_DOWN,  ImGuiKey_DownArrow);
    MapKey(GameInputKey::KEY_ENTER,     ImGuiKey_Enter);
    MapKey(GameInputKey::KEY_ESCAPE,    ImGuiKey_Escape);
    MapKey(GameInputKey::KEY_BACKSPACE, ImGuiKey_Backspace);
    MapKey(GameInputKey::KEY_TAB,       ImGuiKey_Tab);

    MapKey(GameInputKey::KEY_A, ImGuiKey_A);
    MapKey(GameInputKey::KEY_C, ImGuiKey_C);
    MapKey(GameInputKey::KEY_V, ImGuiKey_V);
    MapKey(GameInputKey::KEY_X, ImGuiKey_X);
    MapKey(GameInputKey::KEY_Y, ImGuiKey_Y);
    MapKey(GameInputKey::KEY_Z, ImGuiKey_Z);

}


void MyGui::RenderFrame() {
    if (!initialized) {
        return;
    }

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}
