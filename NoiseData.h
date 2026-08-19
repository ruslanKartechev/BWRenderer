#pragma once
#include "MyTypes.h"
#include <vector>
struct NoiseData {
    i32 sizeX;
    i32 sizeY;
    i32 octaves = 8;
    f32 scale;
    i32 arraySize;
    bool isGenerated;
    bool updated;
    // array of actual noise values
    std::vector<f32> dataPtr = {};
    i32 seed = 1;
    f32 persistence;

    // Used for 'sweeping' as camera moves
    i32 sourceCenterX = 0;
    i32 sourceCenterY = 0;

    i32 windowMaxY = 0;
    i32 windowMinY = 0;
    i32 windowMaxX = 0;
    i32 windowMinX = 0;


    bool FreeData() {
        if (isGenerated) {
            isGenerated = false;
            dataPtr.clear();
            arraySize = 0;
            return true;
        }
        return false;
    }
};
