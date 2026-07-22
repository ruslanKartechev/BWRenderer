#include "Terrain.h"
#include <iostream>
#include <cmath>
#include <cstdlib>

// #define LOG




void Terrain::GenerateMeshData() {
    constexpr int cellsX = 100;
    constexpr int cellsY = 100;

    constexpr int vertexCountX = cellsX + 1;
    constexpr int vertexCountY = cellsY + 1;

    constexpr int totalVertices = vertexCountX * vertexCountY;
    constexpr int totalCells = cellsX * cellsY;

    constexpr float stepX = 1.0f;
    constexpr float stepY = 1.0f;

    constexpr int DATA_COUNT = (2 + 3) * totalVertices;
    constexpr int TRIG_IND_COUNT = totalCells * 6;

    float offsetX = -1.0 * vertexCountX * .5f * stepX;
    float offsetY = -1.0 * vertexCountY * .5f * stepY;

    this->vertexData = new float[DATA_COUNT];
    this->indexData = new int[TRIG_IND_COUNT];
    this->vertexDataCount = DATA_COUNT;
    this->indexCount = TRIG_IND_COUNT;
    this->cellsCountX = cellsX;
    this->cellsCountY = cellsY;

    this->stride = 5;
    this->startIndexVertex = 0;
    this->startIndexUV = 3;
    this->startIndexNormals = -1;
    this->startIndexTangent = -1;
    this->startIndexColor = -1;

    size_t vIdx = 0;
    for (size_t y = 0; y < vertexCountY; y++) {
        float v = (float)y / cellsY;

        for (size_t x = 0; x < vertexCountX; x++) {

            float coordX = x * stepX + offsetX;
            float coordY = 0.0f;
            float coordZ = y * stepY + offsetY;

            float u = (float)x / cellsX;
            vertexData[vIdx++] = coordX;
            vertexData[vIdx++] = coordY;
            vertexData[vIdx++] = coordZ;
            vertexData[vIdx++] = u;
            vertexData[vIdx++] = v;
        }
    }

    size_t iIdx = 0;
    for (size_t y = 0; y < cellsY; y++) {
        for (size_t x = 0; x < cellsX; x++) {
            int topLeft = y * vertexCountX + x;
            int topRight = topLeft + 1;
            int bottomLeft = (y + 1) * vertexCountX + x;
            int bottomRight = bottomLeft + 1;

            indexData[iIdx++] = topLeft;
            indexData[iIdx++] = bottomLeft;
            indexData[iIdx++] = topRight;

            indexData[iIdx++] = topRight;
            indexData[iIdx++] = bottomLeft;
            indexData[iIdx++] = bottomRight;
        }
    }
    this->isBuilt = true;


#ifdef LOG
    size_t idx = 0;
    std::cout << "------------------" << std::endl;
    for (size_t i = 0; i < vertexDataCount; i += stride) {
        std::cout << vertexData[idx++] << " ,  ";
        std::cout << vertexData[idx++] << " ,  ";
        std::cout << vertexData[idx++] << " ,  ";
        std::cout << vertexData[idx++] << " ,  ";
        std::cout << vertexData[idx++] << " ,  ";
        std::cout << std::endl;
    }


    std::cout << "------------------" << std::endl;
    idx = 0;
    for (size_t i = 0; i < indexCount; i += 6) {
        std::cout << indexData[idx++] << " ,  ";
        std::cout << indexData[idx++] << " ,  ";
        std::cout << indexData[idx++] << " ,  ";

        std::cout << indexData[idx++] << " ,  ";
        std::cout << indexData[idx++] << " ,  ";
        std::cout << indexData[idx++] << " ,  ";

        std::cout << std::endl;
    }
#endif
}

void Terrain::FreeData() {
    if (vertexData != nullptr) {
        delete[](vertexData);
    }

    if (indexData != nullptr) {
        delete[](indexData);
    }
}

float* Terrain::GetVertexDataPtr() {

    return vertexData;
}

int* Terrain::GetIndexDataPtr() {

    return indexData;
}