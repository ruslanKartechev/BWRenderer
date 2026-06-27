#ifndef RENDERGL_GAMETIME_H
#define RENDERGL_GAMETIME_H
#include "MyTypes.h"

void Time_Init();
void Time_SetTargetFrameRate(int framesPerSecond);

u64 Time_GetFrameCount();
u32 Time_GetFrameCountInt();

/// @return  Total Time in seconds since startup
f64 Time_GetTotal();

/// @return  Delta Time between frames in Seconds
f64 Time_GetDelta();

void Time_Update();

#endif //RENDERGL_GAMETIME_H
