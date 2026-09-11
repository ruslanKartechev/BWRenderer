#include "NoiseGenerator.h"
#include "Handle.h"
#include "Engine.h"
#include <iostream>
#include <cmath>

#define maxPrimeIndex 10
static constexpr f64 PI = 3.1415927;

static f64 persistence = 0.25;
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

static f32 SmoothStep(f32 edge0, f32 edge1, f32 x) {
    f32 t = std::clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
    return t * t * (3.0f - 2.0f * t);
}


static inline int TrueMod(int coord, int size) {
    int shiftedValue = coord + (size / 2);
    return (shiftedValue % size + size) % size;
}



f64 Noise(int i, int x, int y) {
    int n = x + y * 57;
    n = (n << 13) ^ n;
    int a = primes[i][0], b = primes[i][1], c = primes[i][2];
    int t = (n * (n * n * a + b) + c) & 0x7fffffff;
    return 1.0 - (f64)(t)/1073741824.0;
}

f64 SmoothedNoise(int i, int x, int y) {
    f64 corners = (Noise(i, x-1, y-1) + Noise(i, x+1, y-1) + Noise(i, x-1, y+1) + Noise(i, x+1, y+1)) / 16;
    f64  sides = (Noise(i, x-1, y) + Noise(i, x+1, y) + Noise(i, x, y-1) + Noise(i, x, y+1)) / 8;
    f64 center = Noise(i, x, y) / 4;
    return corners + sides + center;
}


f64 Interpolate(f64 a, f64 b, f64 x) {  // cosine interpolation
    f64 f = (1 - cos(x * PI)) * 0.5;
    return  a * (1 - f) + b * f;
}


f64 Interpolate2DNoise(i32 i, f64 x, f64 y) {
    i32 integer_X = (i32)std::floor(x);
    f64 fractional_X = x - integer_X;
    i32 integer_Y = (i32)std::floor(y);
    f64 fractional_Y = y - integer_Y;

    f64 v1 = SmoothedNoise(i, integer_X, integer_Y);
    f64 v2 = SmoothedNoise(i, integer_X + 1, integer_Y);
    f64 v3 = SmoothedNoise(i, integer_X, integer_Y + 1);
    f64 v4 = SmoothedNoise(i, integer_X + 1, integer_Y + 1);

    f64 i1 = Interpolate(v1, v2, fractional_X);
    f64 i2 = Interpolate(v3, v4, fractional_X);
    return Interpolate(i1, i2, fractional_Y);
}


NoiseGrad NoiseGenerator::NoiseWithDerivatives(i32 primeIndexOffset, f64 x, f64 y) {
    NoiseGrad output = {};
    i32 ix = (int)std::floor(x);
    i32 iy = (int)std::floor(y);
    f64 fx = x - ix;
    f64 fy = y - iy;
    f64 a = SmoothedNoise(primeIndexOffset, ix, iy);
    f64 b = SmoothedNoise(primeIndexOffset, ix + 1, iy);
    f64 c = SmoothedNoise(primeIndexOffset, ix, iy + 1);
    f64 d = SmoothedNoise(primeIndexOffset, ix + 1, iy + 1);

    f64 u = fx * fx * (3.0 - 2.0 * fx); // using 3rd degree polyn.
    f64 v = fy * fy * (3.0 - 2.0 * fy); // using 3rd degree polyn.
    f64 du = 6.0 * fx * (1.0f - fx); // analytical derivative
    f64 dv = 6.0 * fy * (1.0f - fy); // analytical derivative
    // a + (b-a)u + (c-a)v + (a-b-c+d)uv : Bilinear interpolation simplification
    f64 k0 = a;
    f64 k1 = b - a;
    f64 k2 = c - a;
    f64 k3 = a - b - c + d;
    output.val = k0 + k1 * u + k2 * v + k3 * u * v;
    output.dx = (k1 + k3 * v) * du;
    output.dy = (k2 + k3 * u) * dv;

    output.val = (output.val + 1.0) * 0.5;
    output.dx *= 0.5;
    output.dy *= 0.5;

    return output;
}



f64 NoiseGenerator::RidgedNoise_2D(f64 x, f64 y, i32 octaves) {
    f64 total = 0;
    f64 frequency = pow(2, octaves);

    f64 amplitude = 1;
    f64 weight = 1.0;
    f64 maxAmp = 0.0;
    for (int i = 0; i < octaves; ++i) {
        frequency /= 2;
        f64 n = Interpolate2DNoise((primeIndex + i) % maxPrimeIndex, x / frequency, y / frequency);
        n = 1.0 - std::abs(n);
        n *= n;
        total += n * amplitude * weight;
        maxAmp += amplitude * weight;

        amplitude *= persistence;
        weight = std::clamp(n, 0.0, 1.0);
    }

    return total / maxAmp;
}


f64 NoiseGenerator::ValueNoise_2D(f64 x, f64 y, i32 octaves) {
    f64 total = 0.0f;
    f64 frequency = pow(2, octaves);
    f64 amplitude = 1.0f;
    for (int i = 0; i < octaves; ++i) {
        frequency /= 2.0f;
        amplitude *= persistence;
        total += Interpolate2DNoise((primeIndex + i) % maxPrimeIndex, x / frequency, y / frequency) * amplitude;
    }
    return total;
}


f64 NoiseGenerator::GradientNoise2D(f64 x, f64 y, i32 octaves, f32 erosionFactor) {
    f64 total = 0.0f;
    f64 frequency = pow(2, octaves);
    f64 amplitude = 1.0f;
    f64 maxAmplitude = 0.0f;
    f64 dx = 0.0f;
    f64 dy = 0.0f;

    for (int i = 0; i < octaves; ++i) {
        auto data = NoiseWithDerivatives((primeIndex + i) % maxPrimeIndex, x / frequency, y / frequency);
        frequency /= 2.0f;
        dx += data.dx;
        dy += data.dy;
        f64 slope = dx * dx + dy * dy;
        f64 newAmplitude = amplitude / (1.0 + erosionFactor + slope);
        total += data.val * newAmplitude;

        maxAmplitude += newAmplitude;
        amplitude *= persistence;
    }
    return total / maxAmplitude;
}



f64 NoiseGenerator::LayerNoise2D(f64 x, f64 y, std::vector<NoiseLayer>& layers) {
    f64 total = 0;
    imax size = layers.size();
    for (imax i = 0; i < size; i++) {

        total += Interpolate2DNoise((primeIndex + (i32)i) % maxPrimeIndex, x / layers[i].frequency, y / layers[i].frequency) * layers[i].amplitude;
    }
    return total;
}


bool NoiseGenerator::GenerateTerrainNoise(NoiseData& data) {

    primeIndex = data.seed;
    persistence = data.persistence;

    if (data.sizeY == 0 && data.sizeX == 0) {
        std::cerr << "[noise generator] size is 0" << std::endl;
        return false;
    }
    FreeDataNoise(data);
    data.arraySize = data.sizeY * data.sizeX;
    data.dataPtr.resize(data.arraySize);

    // Initialize bounds
    data.windowMaxY = data.sizeY / 2 - 1;
    data.windowMinY = -data.sizeY / 2;
    data.windowMaxX = data.sizeX / 2 - 1;
    data.windowMinX = -data.sizeX / 2;
    // data.sourceCenterX = 0;
    // data.sourceCenterY = 0;
    f64 integ;
    switch (data.noiseType) {
        case NoiseData::TYPE_PERLIN_SIMPLE: {
            for (i32 y = data.windowMinY; y <= data.windowMaxY; y++) {
                i32 bufferY = TrueMod(y, data.sizeY);
                i32 startIdx = bufferY * data.sizeX;
                for (i32 x = data.windowMinX; x <= data.windowMaxX; x++) {

                    f64 val = ValueNoise_2D(x, y, data.octaves);
                    val = (val + 1.0) / 2.0;
                    // val = std::modf(val, &integ);
                    i32 bufferX = TrueMod(x, data.sizeX);
                    data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
                }
            }
            break;
        }
        case NoiseData::TYPE_PERLIN_RIDGED: {
            for (i32 y = data.windowMinY; y <= data.windowMaxY; y++) {
                i32 bufferY = TrueMod(y, data.sizeY);
                i32 startIdx = bufferY * data.sizeX;
                for (i32 x = data.windowMinX; x <= data.windowMaxX; x++) {

                    f64 val = RidgedNoise_2D(x, y, data.octaves);
                    i32 bufferX = TrueMod(x, data.sizeX);
                    data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
                }
            }
            break;
        }
        case NoiseData::TYPE_DOMAIN_WARP: {
            const f64 warpOffset = 5.2;
            const f64 warpStrength = 15.0;
            for (i32 y = data.windowMinY; y <= data.windowMaxY; y++) {
                i32 bufferY = TrueMod(y, data.sizeY);
                i32 startIdx = bufferY * data.sizeX;
                for (i32 x = data.windowMinX; x <= data.windowMaxX; x++) {

                    f64 warpX = ValueNoise_2D(x, y, 3) * warpStrength;
                    f64 warpY = ValueNoise_2D(x + warpOffset, y + warpOffset, 3) * warpStrength;
                    f64 val = ValueNoise_2D(x + warpX, y + warpY, data.octaves);
                    val = (val + 1.0) / 2.0;

                    i32 bufferX = TrueMod(x, data.sizeX);
                    data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
                }
            }
            break;
        }
        case NoiseData::TYPE_PERLIN_LAYERED: {
            // TODO: Implement custom layers and weights
            for (i32 y = data.windowMinY; y <= data.windowMaxY; y++) {
                i32 bufferY = TrueMod(y, data.sizeY);
                i32 startIdx = bufferY * data.sizeX;
                for (i32 x = data.windowMinX; x <= data.windowMaxX; x++) {

                    // f64 val = RidgedNoise_2D(x, y, data.octaves);
                    // val = (val + 1.0) / 2.0;
                    // val = std::modf(val, &integ);
                    // i32 bufferX = TrueMod(x, data.sizeX);
                    // data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
                }
            }
            break;
        }
        case NoiseData::TYPE_DERIVATIVES: {
            for (i32 y = data.windowMinY; y <= data.windowMaxY; y++) {
                i32 bufferY = TrueMod(y, data.sizeY);
                i32 startIdx = bufferY * data.sizeX;
                for (i32 x = data.windowMinX; x <= data.windowMaxX; x++) {

                    f64 val = GradientNoise2D(x, y, data.octaves, data.erosionFactor);
                    // val = (val + 1.0) / 2.0;
                    i32 bufferX = TrueMod(x, data.sizeX);
                    data.dataPtr[startIdx + bufferX] = static_cast<f32>(val);
                }
            }
            break;
        }


    }


    data.isGenerated = true;
    return true;
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
            f64 val = ValueNoise_2D(worldX, worldY, data.octaves);
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
            f64 val = ValueNoise_2D(worldX, worldY, data.octaves);
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
            f64 val = ValueNoise_2D(worldX, worldY, data.octaves);
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
            f64 val = ValueNoise_2D(worldX, worldY, data.octaves);
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

