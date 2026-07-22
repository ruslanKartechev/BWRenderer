#pragma once
#include "MyTypes.h"

class NoiseGenerator{
public:

    i32 sizeX;
    i32 sizeY;
    i32 octaves = 8;

    void GeneratePerlin();
    bool IsGenerated() const;
    bool FreeData();

    f32* GetDataPtr();
    imax GetDataArraySize();


private:
    f32* dataPtr;
    imax arraySize;
    bool isGenerated;

    double ValueNoise_2D(double x, double y);

};

