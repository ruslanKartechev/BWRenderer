#pragma once
#include "MyTypes.h"

struct NoiseData {
    i32 sizeX;
    i32 sizeY;
    i32 octaves = 8;
    f32 scale;
    i32 arraySize;
    bool isGenerated;
    // array of actual noise values
    f32* dataPtr;
    i32 seed;
    f32 persistence;


    bool FreeData() {
        if (isGenerated) {
            isGenerated = false;
            if (dataPtr != nullptr) {
                delete[] dataPtr;
                arraySize = 0;
                return true;
            }
        }
        return false;
    }
};
