#include "NoiseGenerator.h"
#include "Handle.h"
#include "Engine.h"
#include <iostream>

#define maxPrimeIndex 10
static double persistence = 0.25;
static int primeIndex = 1;

static int primes[maxPrimeIndex][3] = {
    { 995615039, 600173719, 701464987 },
    { 831731269, 162318869, 136250887 },
    { 174329291, 946737083, 245679977 },
    { 362489573, 795918041, 350777237 },
    { 457025711, 880830799, 909678923 },
    { 787070341, 177340217, 593320781 },
    { 405493717, 291031019, 391950901 },
    { 458904767, 676625681, 424452397 },
    { 531736441, 939683957, 810651871 },
    { 997169939, 842027887, 423882827 }
};

double Noise(int i, int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int a = primes[i][0], b = primes[i][1], c = primes[i][2];
    int t = (n * (n * n * a + b) + c) & 0x7fffffff;
    return 1.0 - (double)(t)/1073741824.0;
}

double SmoothedNoise(int i, int x, int y) {
    double corners = (Noise(i, x-1, y-1) + Noise(i, x+1, y-1) +
                      Noise(i, x-1, y+1) + Noise(i, x+1, y+1)) / 16,
           sides = (Noise(i, x-1, y) + Noise(i, x+1, y) + Noise(i, x, y-1) +
                    Noise(i, x, y+1)) / 8,
           center = Noise(i, x, y) / 4;
    return corners + sides + center;
}

double Interpolate(double a, double b, double x) {  // cosine interpolation
    double ft = x * 3.1415927,
           f = (1 - cos(ft)) * 0.5;
    return  a*(1-f) + b*f;
}

double InterpolatedNoise(int i, double x, double y) {
    int integer_X = x;
    double fractional_X = x - integer_X;
    int integer_Y = y;
    double fractional_Y = y - integer_Y;

    double v1 = SmoothedNoise(i, integer_X, integer_Y),
           v2 = SmoothedNoise(i, integer_X + 1, integer_Y),
           v3 = SmoothedNoise(i, integer_X, integer_Y + 1),
           v4 = SmoothedNoise(i, integer_X + 1, integer_Y + 1),
           i1 = Interpolate(v1, v2, fractional_X),
           i2 = Interpolate(v3, v4, fractional_X);
    return Interpolate(i1, i2, fractional_Y);
}



static f32 SmoothStep(f32 edge0, f32 edge1, f32 x) {
    f32 t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}



bool NoiseGenerator::GenerateSplatMapForTerrain(SplatMapData& splat, NoiseData& terrainNoise, f32 heightPower)
{
    if (terrainNoise.dataPtr.empty()) {
        std::cerr << "[NoiseGen] TerrainNoise.dataPtr is null!" << std::endl;
        return false;
    }
    splat.Clear();

    i32 channels = splat.stride;
    if (channels == 0) {
        splat.stride = channels = 4;
    }

    splat.sizeX = terrainNoise.sizeX;
    splat.sizeY = terrainNoise.sizeY;
    i32 totalSize = terrainNoise.arraySize * channels;
    i32 readArraySize = terrainNoise.arraySize;
    f32 hScale = terrainNoise.scale;
    splat.arraySize = totalSize;
    splat.dataPtr = new u8[totalSize];


    f32 maxPossibleHeight = std::pow(0.8 * hScale, heightPower);

    for (size_t i = 0; i < readArraySize; i++) {
        f32 hRaw = terrainNoise.dataPtr[i];
        f32 hVis = std::pow(hRaw * hScale, heightPower);
        // relative height (0-1)
        f32 rh = hVis / maxPossibleHeight;

        // Weights
        f32 wRock1 = 0.0f; // Deep
        f32 wSand  = 0.0f; // Low ground
        f32 wGrass = 0.0f; // Mid ground
        f32 wRock2 = 0.0f; // Peaks
        if (rh < splat.band1) {
            wRock1 = 1.0;
        }
        else if (rh < splat.band2) {
            wSand = 1.0;
        }
        else if (rh < splat.band3) {
            wGrass = 1.0;
        }
        else if (rh < splat.band4) {
            wRock2 = 1.0;
        }

        /* R */ splat.dataPtr[i * channels + 0] = static_cast<u8>(wRock1 * 255.0f);
        /* G */ splat.dataPtr[i * channels + 1] = static_cast<u8>(wSand * 255.0f);
        /* B */ splat.dataPtr[i * channels + 2] = static_cast<u8>(wGrass * 255.0f);
        /* A */ splat.dataPtr[i * channels + 3] = static_cast<u8>(wRock2 * 255.0f);
    }

    splat.isGenerated = true;

    return true;
}



double NoiseGenerator::ValueNoise_2D(double x, double y, i32 octaves) {
    double total = 0;
    double frequency = pow(2, octaves);
    double amplitude = 1;
    for (int i = 0; i < octaves; ++i) {
        frequency /= 2;
        amplitude *= persistence;
        total += InterpolatedNoise((primeIndex + i) % maxPrimeIndex, x / frequency, y / frequency) * amplitude;
    }
    return total / frequency;
}



bool NoiseGenerator::GeneratePerlin(NoiseData& data) {

    primeIndex = data.seed;
    persistence = data.persistence;

    if (data.sizeY == 0 && data.sizeX == 0) {
        std::cerr << "[noise generator] size is 0" << std::endl;
        return false;
    }
    FreeDataNoise(data);

    data.arraySize = data.sizeY * data.sizeX;
    data.dataPtr.resize(data.arraySize);
    size_t arrIdx = 0;
    for (imax y = 0; y < data.sizeY; y++) {
        for (imax x = 0; x < data.sizeX; x++) {

            double val = ValueNoise_2D(x, y, data.octaves);
            val = (val + 1.0) / 2.0;
            data.dataPtr[arrIdx] = static_cast<f32>(val);
            arrIdx++;
        }
    }
    data.windowMaxY = data.sizeY / 2 - 1;
    data.windowMinY = -data.sizeY / 2;
    data.windowMaxX = data.sizeX / 2 - 1;
    data.windowMinX = -data.sizeX / 2;

    data.isGenerated = true;
    return true;
}

static inline int TrueMod(int coord, int size) {
    int shiftedValue = coord + (size / 2);
    return (shiftedValue % size + size) % size;
}

void NoiseGenerator::GenerateAdditionalPerlin(NoiseData& data, f32 cameraX, f32 cameraY) {
    if (data.dataPtr.empty()) {
        printf("ERROR null \n");
        return;
    }
    i32 flooredX = floorl(cameraX);
    i32 flooredY = floorl(cameraY);
    i32 deltaX = flooredX - data.sourceCenterX;
    i32 deltaY = flooredY - data.sourceCenterY;

    if (deltaX == 0 && deltaY == 0) {
        return; // No movement, skip generation
    }

    // Y MOVEMENT (Update Rows)
    // Moving Forward
    while (data.sourceCenterY < flooredY) {
        data.sourceCenterY++;
        data.windowMaxY++;
        data.windowMinY++;

        i32 worldY = data.windowMaxY;
        i32 bufferY = TrueMod(worldY, data.sizeY);
        i32 startIdx = bufferY * data.sizeX;

        printf("moving forward : %d , start idx: %d \n", flooredY, startIdx);
        // Iterate exactly sizeX times across the current window
        for (i32 i = 0; i < data.sizeX; i++) {
            i32 worldX = data.windowMinX + i;
            double val = ValueNoise_2D(worldX, worldY, data.octaves);
            val = (val + 1.0) / 2.0;
            i32 bufferX = TrueMod(worldX, data.sizeX);
            f32 prevValue = data.dataPtr[startIdx + bufferX];
            data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
        }
        data.updated = true;
    }

    // Moving Backward
    while (data.sourceCenterY > flooredY) {
        data.sourceCenterY--;
        data.windowMinY--;
        data.windowMaxY--;

        i32 worldY = data.windowMinY;
        i32 bufferY = TrueMod(worldY, data.sizeY);
        i32 startIdx = bufferY * data.sizeX;

        for (i32 i = 0; i < data.sizeX; i++) {
            i32 worldX = data.windowMinX + i;
            double val = ValueNoise_2D(worldX, worldY, data.octaves);
            val = (val + 1.0) / 2.0;
            i32 bufferX = TrueMod(worldX, data.sizeX);
            data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
        }
        data.updated = true;
    }

    // X MOVEMENT (Update Columns)
    // Right
    while (data.sourceCenterX < flooredX) {
        data.sourceCenterX++;
        data.windowMaxX++;
        data.windowMinX++;

        i32 worldX = data.windowMaxX;
        i32 bufferX = TrueMod(worldX, data.sizeX);

        for (i32 i = 0; i < data.sizeY; i++) {
            i32 worldY = data.windowMinY + i;
            double val = ValueNoise_2D(worldX, worldY, data.octaves);
            val = (val + 1.0) / 2.0;
            i32 bufferY = TrueMod(worldY, data.sizeY);
            i32 startIdx = bufferY * data.sizeX;
            data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
        }
        data.updated = true;
    }
    // Left
    while (data.sourceCenterX > flooredX) {
        data.sourceCenterX--;
        data.windowMinX--;
        data.windowMaxX--;
        i32 worldX = data.windowMinX;
        i32 bufferX = TrueMod(worldX, data.sizeX);

        for (i32 i = 0; i < data.sizeY; i++) {
            i32 worldY = data.windowMinY + i;
            double val = ValueNoise_2D(worldX, worldY, data.octaves);
            val = (val + 1.0) / 2.0;
            i32 bufferY = TrueMod(worldY, data.sizeY);
            i32 startIdx = bufferY * data.sizeX;
            data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
        }
        data.updated = true;
    }
}




bool NoiseGenerator::FreeDataSplat(SplatMapData& map) {
    if (map.isGenerated) {
        map.isGenerated = false;

        if (map.dataPtr != nullptr) {
            delete[] map.dataPtr;
            map.arraySize = 0;
            return true;
        }
    }
    return false;
}



bool NoiseGenerator::FreeDataNoise(NoiseData& noise) {
    if (noise.isGenerated) {
        noise.isGenerated = false;
        noise.dataPtr.clear();
        return true;
    }
    return false;
}