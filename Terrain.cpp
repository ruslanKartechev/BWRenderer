#include "Terrain.h"
#include <iostream>
#include <cmath>


void Terrain::GenerateStrip(ClipmapMesh& mesh, int length, float stepX, float stepZ, bool reversTriangles) {
    assert(mesh.vertexDataPtr == nullptr);
    assert(mesh.indexDataPtr == nullptr);

    mesh.stride = 2;
    mesh.vertexDataCount = 2 * length * mesh.stride;
    mesh.indexCount = (length - 1) * 3;

    mesh.vertexDataPtr = new f32[mesh.vertexDataCount];
    mesh.indexDataPtr = new i32[mesh.indexCount];

    i32 vIdx = 0;
    if (stepX > stepZ) {
        for (auto i = 0; i < length; i++) {
            mesh.vertexDataPtr[vIdx++] = (f32)i * stepX; // P1 X Coord
            mesh.vertexDataPtr[vIdx++] = (f32)0 * stepZ; // P1 Z Coord
            mesh.vertexDataPtr[vIdx++] = (f32)i * stepX; // P2 X Coord
            mesh.vertexDataPtr[vIdx++] = (f32)1 * stepZ; // P2 Z Coord
        }
    }
    else {
        for (auto i = 0; i < length; i++) {
            mesh.vertexDataPtr[vIdx++] = (f32)0 * stepX; // P1 X Coord
            mesh.vertexDataPtr[vIdx++] = (f32)i * stepZ; // P1 Z Coord
            mesh.vertexDataPtr[vIdx++] = (f32)1 * stepX; // P2 X Coord
            mesh.vertexDataPtr[vIdx++] = (f32)i * stepZ; // P2 Z Coord
        }
    }

    i32 iIdx = 0;
    if (reversTriangles) {
        for (auto i = 0; i < length-1; i++) {
            if (i % 2 == 0) {
                mesh.indexDataPtr[iIdx++] = i+1;
                mesh.indexDataPtr[iIdx++] = i+2;
                mesh.indexDataPtr[iIdx++] = i+0;
            }
            else {
                mesh.indexDataPtr[iIdx++] = i+2;
                mesh.indexDataPtr[iIdx++] = i+1;
                mesh.indexDataPtr[iIdx++] = i+0;
            }
        }
    }
    else {
        for (auto i = 0; i < length-1; i++) {
            if (i % 2 == 0) {
                mesh.indexDataPtr[iIdx++] = i+0;
                mesh.indexDataPtr[iIdx++] = i+2;
                mesh.indexDataPtr[iIdx++] = i+1;
            }
            else {
                mesh.indexDataPtr[iIdx++] = i+0;
                mesh.indexDataPtr[iIdx++] = i+1;
                mesh.indexDataPtr[iIdx++] = i+2;
            }
        }
    }
}


void Terrain::GenerateGridNoUV(ClipmapMesh& mesh, int verticesX, int verticesZ, float stepX, float stepZ) {
    assert(mesh.vertexDataPtr == nullptr);
    assert(mesh.indexDataPtr == nullptr);

    mesh.stride = 2;
    mesh.vertexDataCount = verticesX * verticesZ * mesh.stride;
    mesh.indexCount = (verticesX - 1) * (verticesZ - 1) * 6;

    mesh.vertexDataPtr = new f32[mesh.vertexDataCount];
    mesh.indexDataPtr = new i32[mesh.indexCount];


    i32 vIdx = 0;
    for (auto z = 0; z < verticesZ; z++) {
        for (auto x = 0; x < verticesX; x++) {
            mesh.vertexDataPtr[vIdx++] = (f32)x * stepX; // X Coord
            mesh.vertexDataPtr[vIdx++] = (f32)z * stepZ; // Z Coord
        }
    }

    i32 iIdx = 0;
    for (auto z = 0; z < verticesZ - 1; z++) {
        for (auto x = 0; x < verticesX - 1; x++) {
            i32 topLeft = z * verticesX + x;
            i32 topRight = topLeft + 1;
            i32 bottomLeft = (z + 1) * verticesX + x;
            i32 bottomRight = bottomLeft + 1;

            mesh.indexDataPtr[iIdx++] = topLeft;
            mesh.indexDataPtr[iIdx++] = bottomLeft;
            mesh.indexDataPtr[iIdx++] = topRight;
            mesh.indexDataPtr[iIdx++] = topRight;
            mesh.indexDataPtr[iIdx++] = bottomLeft;
            mesh.indexDataPtr[iIdx++] = bottomRight;
        }
    }
    printf("----Index IDX %d, INDEX COUNT %d\n", iIdx, mesh.indexCount);
}


// Low Exclusive, High inclusive
static i32 ClampCircle(i32 input, i32 low, i32 high) {
    while (input < low) {
        input += high;
    }
    while (input >= high) {
        input -= high;
    }
    return input;
}

f32 Terrain::GetHeightAt(f32 localX, f32 localZ) {
    if (heightData.dataPtr.empty()) {
        return 0.0f;
    }
    f32 halfSize = heightData.sizeX * 0.5f;
    i32 pz = ClampCircle( std::floor(localZ + halfSize), 0, heightData.sizeX);
    i32 px = ClampCircle( std::floor(localX + halfSize), 0, heightData.sizeX);

    i32 idx = static_cast<i32>(std::lround(px + pz * heightData.sizeX));
    if (idx >= heightData.dataPtr.size() || idx < 0) {
        std::cerr << "index  (" << idx << ")  out of range: (0, " << heightData.arraySize << std::endl;
    }
    idx = ClampCircle(idx, 0, heightData.arraySize);

    f32 h = heightData.dataPtr[idx];
    h = pow(h * heightData.scale, HeightPower) - pow(0.5 * heightData.scale, HeightPower);
    // printf("Position Local (%f, %f)  INDEX %d   H: %f\n", localX, localZ, idx, h);
    return h;
}


void Terrain::GetVertexOriginPosition(vec3 worldPosition, vec3 outPosition)
{
    glm_vec3_copy(worldPosition, outPosition);
    outPosition[0] -= (worldSize + 1.0) * 0.5f;
    outPosition[2] -= (worldSize + 1.0) * 0.5f;
}



void Terrain::GenerateSingleGrid() {
    i32 vertexCountX = worldSize + 1;
    i32 vertexCountY = worldSize + 1;
    GenerateGridNoUV(singleMesh, vertexCountX, vertexCountY);
}


void Terrain::GenerateMeshData() {

    constexpr i32 n = 255; // 14 units
    constexpr i32 ring = n + 2; // 16 units For vertical strips
    constexpr i32 shortRing = n; // 14 units For horizontal strips

    constexpr i32 m = (n + 1) / 4; // vertices in the square MxM block
    constexpr i32 gapWidth = (n-1) - 4 * (m-1) + 1; // N - 4M units wide (+1 vertex)
    constexpr i32 centerVertCount = ((n+1)/2) + 1; // half of N (+1 vertex)

    LODS = 4;
    R = (n - 1) / 2;
    M = m - 1;
    Gap = gapWidth - 1;

    GenerateGridNoUV(centerMesh, centerVertCount, centerVertCount);
    GenerateGridNoUV(blockMesh, m, m);
    GenerateGridNoUV(fixUpXMesh, m, gapWidth);
    GenerateGridNoUV(fixUpZMesh, gapWidth, m);

    GenerateGridNoUV(trimXMesh, shortRing, 2);
    GenerateGridNoUV(trimZMesh, 2, ring);

    constexpr f32 w = 0.0f;
    constexpr f32 step = 0.5f;
    GenerateStrip(zeroAreaX1, ring * 4, step, w, true);
    GenerateStrip(zeroAreaX2, ring * 4, step, w, true);
    GenerateStrip(zeroAreaZ1, ring * 4, w, step, false);
    GenerateStrip(zeroAreaZ2, ring * 4, w, step, false);

    centerMesh.sizeX = centerMesh.sizeZ = centerVertCount-1;

    blockMesh.sizeX = blockMesh.sizeZ = M;

    fixUpXMesh.sizeX = M;
    fixUpXMesh.sizeZ = Gap;

    fixUpZMesh.sizeX = Gap;
    fixUpZMesh.sizeZ = M;

    trimXMesh.sizeX = shortRing-1;
    trimXMesh.sizeZ = 1;

    trimZMesh.sizeX = 1;
    trimZMesh.sizeZ = shortRing-1;

    zeroAreaX1.sizeX = zeroAreaX1.sizeX = ring * 4 - 1;
    zeroAreaX1.sizeZ = zeroAreaX1.sizeZ = 1;

    zeroAreaZ1.sizeX = zeroAreaZ1.sizeX = 1;
    zeroAreaZ1.sizeZ = zeroAreaZ1.sizeZ = ring * 4 - 1;

    this->isBuilt = true;
}


void Terrain::FreeData() {

}
