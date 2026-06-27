#include "GameTime.h"
#include <windows.h>
#include <timeapi.h> // Required for timeBeginPeriod
#include "MyTypes.h"

static u64 s_perfFrequency = 0.0;
static u64 s_startCounter = 0;
static u64 s_lastCounter = 0;
static u64 s_framesCount = 0;

static double s_delta;
static double s_totalTimeSinceStart;
static double s_targetRate;
static bool s_capped;

u64 Time_GetFrameCount() {
    return s_framesCount;
}

u32 Time_GetFrameCountInt() {
    return (u32)s_framesCount;
}

void Time_Init() {
    LARGE_INTEGER li;
    QueryPerformanceFrequency(&li);
    s_perfFrequency = li.QuadPart;
    timeBeginPeriod(1); // 1 ms

    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);
    s_startCounter = counter.QuadPart;
    s_lastCounter = counter.QuadPart;
}


void Time_SetTargetFrameRate(int framesPerSecond) {
    if (framesPerSecond <= 0) {
        s_capped = false;
        return;
    }
    s_capped = true;
    s_targetRate = 1.0 / ((double)framesPerSecond);
}

/// @return  Total Time in seconds since startup
f64 Time_GetTotal() {
    return s_totalTimeSinceStart;
}

f64 Time_GetDelta() {
    return s_delta;
}


static double GetTimeNow() {
    LARGE_INTEGER current;
    QueryPerformanceCounter(&current);
    return (double)current.QuadPart / s_perfFrequency;
}

void Time_Update() {
    f64 time = GetTimeNow();
    f64 elapsed = time - (f64)(s_lastCounter) / (f64)(s_perfFrequency);
    if (s_capped) {
        while (elapsed < s_targetRate) {
            f64 timeRemaining = s_targetRate - elapsed;
            if (timeRemaining > 2) { // ms
                Sleep(1); // ms
            }
            else {
                YieldProcessor();
            }
            time = GetTimeNow();
            elapsed = time - (f64)(s_lastCounter) / (f64)(s_perfFrequency);

        }
    }
    LARGE_INTEGER counter;
    QueryPerformanceCounter(&counter);

    s_delta = (f64)(counter.QuadPart - s_lastCounter) / (f64)(s_perfFrequency);
    s_totalTimeSinceStart = (f64)(counter.QuadPart - s_startCounter) / (f64)(s_perfFrequency);
    s_lastCounter = counter.QuadPart;
    s_framesCount++;
}