#pragma once
#include "ProgramWindow.h"

class MyGui {
public:

    void InitForWindow(ProgramWindow* window);

    void StartFrame();
    void EndFrame();

    void RenderFrame();

    void UpdateInputs();

private:
    bool initialized = false;
    ProgramWindow* windowPtr = nullptr;

};

