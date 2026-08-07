#include "Terrain.h"
#include <iostream>
#include <cmath>



void Terrain::GenerateGridNoUV(ClipmapMesh& mesh, int verticesX, int verticesZ) {
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
            mesh.vertexDataPtr[vIdx++] = (f32)x;
            mesh.vertexDataPtr[vIdx++] = (f32)z;
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
}


f32 Terrain::GetHeightAt(f32 localX, f32 localZ) {
    if (heightData.dataPtr == nullptr) {
        return 0.0f;
    }
    f32 u = localX;
    f32 v = localZ;

    while (u >= heightData.sizeX && heightData.sizeX != 0) {
        u -= heightData.sizeX;
    }
    while (v >= heightData.sizeY && heightData.sizeY != 0) {
        v -= heightData.sizeY;
    }

    i32 idx = static_cast<i32>(std::lround(u + v * heightData.sizeX) );
    if (idx >= heightData.arraySize) {
        std::cerr << "index " << idx << " out of bounce" << heightData.arraySize << std::endl;
        return 0.0f;
    }
    f32 h = heightData.dataPtr[idx];
    // printf("Position Local (%f, %f) -> (u,v) (%f, %f). h: %f\n", localX, localZ, u, v, h);
    h = pow(h * heightData.scale, HeightPower) - pow(0.5 * heightData.scale, HeightPower);
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

    constexpr i32 m = (n + 1) / 4; // 3 by 3 units
    constexpr i32 gapWidth = (n-1) - 4*(m-1) + 1; // N - 4M units wide (+1 vertex)
    constexpr i32 centerVertCount = ((n+1) / 2) + 1; // 8 units wide
    constexpr i32 trimsVertCount = 2;

    LODS = 4;
    R = (n - 1) / 2;
    M = m - 1;
    Gap = gapWidth - 1;

    GenerateGridNoUV(centerMesh, centerVertCount, centerVertCount);
    GenerateGridNoUV(blockMesh, m, m);
    GenerateGridNoUV(fixUpXMesh, m, gapWidth);
    GenerateGridNoUV(fixUpYMesh, gapWidth, m);

    GenerateGridNoUV(trimXMesh, shortRing, trimsVertCount);
    GenerateGridNoUV(trimYMesh, trimsVertCount, ring);

    this->isBuilt = true;
}


void Terrain::FreeData() {

}
