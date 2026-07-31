#pragma once
#include "MyTypes.h"
#include "RenderObject.h"
#include "Handle.h"
#include "NoiseGenerator.h"
#include "SplatMapData.h"
#include "NoiseData.h"

class Terrain {
public:

    MaterialHandle hMaterial = {};
    TransformHandle hTransform = {};

    NoiseData heightData {};
    SplatMapData terrainSplat {};

    TextureHandle hNoiseTex;
    TextureHandle hSplatMapTex;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    f32 width;
    f32 height;

    i32 vertexDataCount;
    i32 indexCount;
    i32 stride;
    i32 startIndexVertex = -1;
    i32 startIndexUV = -1;
    i32 startIndexColor = -1;
    i32 startIndexNormals = -1;
    i32 startIndexTangent = -1;
    i32 cellsCountX;
    i32 cellsCountY;

    bool isBuilt;

    void SetSize(i32 cellsX, i32 cellsY);

    void GenerateMeshData();

    void FreeData();

    float* GetVertexDataPtr();

    int* GetIndexDataPtr();


private:
    f32* vertexData = nullptr;
    i32* indexData = nullptr;

};

