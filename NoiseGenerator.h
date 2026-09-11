#pragma once
#include "MyTypes.h"
#include "SplatMapData.h"
#include "NoiseData.h"

struct NoiseGrad {
    f64 val;
    f64 dx;
    f64 dy;
};

class NoiseGenerator{
public:



    static bool GenerateTerrainNoise(NoiseData& data);
    static void GenerateAdditionalPerlin(NoiseData& data, f32 cameraX, f32 cameraY);

    static bool GenerateSplatMapForTerrain(SplatMapData& splat, NoiseData& terrainNoise, f32 heightPower = 3.0);

    static bool FreeDataSplat(SplatMapData& splat);

    static bool FreeDataNoise(NoiseData& splat);

    static NoiseGrad NoiseWithDerivatives(i32 primeIndexOffset, f64 x, f64 y);

private:

    static f64 ValueNoise_2D(double x, double y, i32 octaves);
    static f64 RidgedNoise_2D(f64 x, f64 y, i32 octaves);
    static f64 LayerNoise2D(double x, double y, std::vector<NoiseLayer>& layers);

    static f64 GradientNoise2D(f64 x, f64 y, i32 octaves, f32 erosionFactor);
};

