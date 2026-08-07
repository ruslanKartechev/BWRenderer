#pragma once
#include "MyTypes.h"
#include "SplatMapData.h"
#include "NoiseData.h"


class NoiseGenerator{
public:
    static bool GeneratePerlin(NoiseData& data);

    static bool GenerateSplatMapForTerrain(SplatMapData& splat, NoiseData& terrainNoise, f32 heightPower = 3.0);

    static bool FreeDataSplat(SplatMapData& splat);

    static bool FreeDataNoise(NoiseData& splat);


private:

    static double ValueNoise_2D(double x, double y, i32 octaves);

};

