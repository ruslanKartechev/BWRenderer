#pragma once
#include "MyTypes.h"
#include <vector>


struct NoiseLayer{
    f32 weight;
    f32 frequency;
    f32 amplitude;

    NoiseLayer(f32 weight, f32 frequency, f32 amplitude) {
        this->weight = weight;
        this->frequency = frequency;
        this-> amplitude = amplitude;
    }
};


struct NoiseData {
    static const i32 TYPE_PERLIN_SIMPLE = 0;
    static const i32 TYPE_PERLIN_RIDGED = 1;
    static const i32 TYPE_PERLIN_LAYERED = 2;
    static const i32 TYPE_DOMAIN_WARP = 3;
    static const i32 TYPE_DERIVATIVES = 4;

    i32 sizeX;
    i32 sizeY;
    i32 octaves = 8;
    f32 scale;
    i32 arraySize;
    f32 erosionFactor = 1.25f;
    bool isGenerated;
    bool updated;
    // array of actual noise values
    std::vector<NoiseLayer> layers = {};
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

    i32 noiseType = 0;

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
