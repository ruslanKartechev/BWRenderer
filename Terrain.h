#pragma once
#include "MyTypes.h"
#include "RenderObject.h"
#include "Handle.h"
#include "SplatMapData.h"
#include "NoiseData.h"

struct ClipmapMesh {
    f32* vertexDataPtr = nullptr;
    i32* indexDataPtr = nullptr;
    GLuint vao = 0;
    GLuint vbo = 0;
    GLuint ebo = 0;
    i32 vertexDataCount = 0;
    i32 indexCount = 0;
    i32 stride = 5;
};



class Terrain {
public:
    static constexpr f32 HeightPower = 3.0f;
    static constexpr f32 SizeMiddle = 7.0f;

    MaterialHandle hMaterial = {};
    TransformHandle hTransform = {};

    NoiseData heightData {};
    SplatMapData terrainSplat {};

    TextureHandle hNoiseTex;
    TextureHandle hSplatMapTex;


    ClipmapMesh singleMesh = {}; // not used

    ClipmapMesh centerMesh = {};
    ClipmapMesh blockMesh = {};
    ClipmapMesh fixUpXMesh = {};
    ClipmapMesh fixUpYMesh = {};
    ClipmapMesh trimXMesh = {};
    ClipmapMesh trimYMesh = {};

    f32 worldSize = 256.0f;

    i32 LODS = 5;
    // Innermost radius in world units
    f32 R = 7.0f;
    // Chunk block size in units
    f32 M = 3.0f;
    // Fix-up block width in units
    f32 Gap = 2.0f;

    i32 textOffsetX = 0;
    i32 textOffsetZ = 0;

    bool isBuilt;
    bool debugSnapping;

    void GenerateMeshData();
    void FreeData();
    f32 GetHeightAt(f32 x, f32 z);
    void GetVertexOriginPosition(vec3 worldPosition, vec3 outPosition);


private:

    static void GenerateGridNoUV(ClipmapMesh& mesh, int verticesX, int verticesZ);
    void GenerateSingleGrid();

};

