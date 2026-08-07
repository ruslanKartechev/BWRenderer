#pragma once
#include "MyTypes.h"

struct SplatMapData {
    i32 sizeX;
    i32 sizeY;

    f32 band1;
    f32 band2;
    f32 band3;
    f32 band4;

    /// How many 'channels' are used to denote different weights
    /// Defaults to 4
    i32 stride = 4;

    /// Array of bytes containing weights
    u8* dataPtr;
    /// Size of 'dataPtr' array
    imax arraySize;

    bool isGenerated;

    void Clear() {
        if (dataPtr != nullptr) {
            delete[] dataPtr;
            arraySize = 0;
            isGenerated = false;
        }
    }
};

