#pragma once
#include "MyTypes.h"

template<typename T, void (T::*Func)(f32)>
void MethodUpdateWrapper(void* ctx, f32 deltaTime) {
    ((T*)(ctx)->*Func) (deltaTime);
}

template<typename T, void (T::*Func)()>
void MethodStartWrapper(void* ctx) {
    ((T*)(ctx)->*Func) ();
}



struct Script{
    void* objectPtr;

    void (*Create)(void* ctx);
    void (*Start)(void* ctx);
    void (*GamePause)(void* ctx);
    void (*GameResume)(void* ctx);
    void (*Quit)(void* ctx);

    void (*Update)(void* ctx, f32 deltaTime);
    void (*PhysicsUpdate)(void* ctx, f32 deltaTime);
    void (*GuiUpdate)(void* ctx, f32 deltaTime);


};
