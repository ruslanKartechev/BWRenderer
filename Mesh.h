#pragma once

typedef struct Mesh{
    float* vertexData = nullptr;
    int* indexData = nullptr;

    int vertexDataCount;
    int indexCount;
    int stride;

    int startIndexVertex = 0;
    int startIndexUV = 0;
    int startIndexColor = 0;
    int startIndexNormals = 0;

} Mesh;




void ClearMesh(Mesh& mesh) {

}


void BuildCubeMesh(Mesh& mesh) {
    constexpr float d = 0.5f;
    mesh.stride = 8;
    mesh.vertexDataCount = 24 * mesh.stride;
    mesh.indexCount = 36;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexNormals = 5;
    mesh.startIndexColor = -1;
    mesh.vertexData = new float[] {
        // POSITION    // UV       // Normal
        // Front Face (Z = 0.5f)
        -d, -d,  d,   0.0f, 0.0f,   0, 0,  1,
         d, -d,  d,   1.0f, 0.0f,   0, 0,  1,
         d,  d,  d,   1.0f, 1.0f,   0, 0,  1,
        -d,  d,  d,   0.0f, 1.0f,   0, 0,  1,
        // Back Face (Z = -d)
         d, -d, -d,   0.0f, 0.0f,   0, 0, -1,
        -d, -d, -d,   1.0f, 0.0f,   0, 0, -1,
        -d,  d, -d,   1.0f, 1.0f,   0, 0, -1,
         d,  d, -d,   0.0f, 1.0f,   0, 0, -1,
        // Left Face (X = -d)
        -d, -d, -d,   0.0f, 0.0f, - 1, 0, 0,
        -d, -d,  d,   1.0f, 0.0f, - 1, 0, 0,
        -d,  d,  d,   1.0f, 1.0f, - 1, 0, 0,
        -d,  d, -d,   0.0f, 1.0f, - 1, 0, 0,
        // Right Face (X = d)
         d, -d,  d,   0.0f, 0.0f,   1, 0, 0,
         d, -d, -d,   1.0f, 0.0f,   1, 0, 0,
         d,  d, -d,   1.0f, 1.0f,   1, 0, 0,
         d,  d,  d,   0.0f, 1.0f,   1, 0, 0,
        // Top Face (Y = d)
        -d,  d,  d,   0.0f, 0.0f,   0, 1, 0,
         d,  d,  d,   1.0f, 0.0f,   0, 1, 0,
         d,  d, -d,   1.0f, 1.0f,   0, 1, 0,
        -d,  d, -d,   0.0f, 1.0f,   0, 1, 0,
        // Bottom Face (Y = -d)
        -d, -d, -d,   0.0f, 0.0f,   0, -1, 0,
         d, -d, -d,   1.0f, 0.0f,   0, -1, 0,
         d, -d,  d,   1.0f, 1.0f,   0, -1, 0,
        -d, -d,  d,   0.0f, 1.0f,   0, -1, 0,
    };

    mesh.indexData = new int[mesh.indexCount] {
        0,  1,  2,    2,  3,  0,  // Front Face
        4,  5,  6,    6,  7,  4,  // Back Face
        8,  9,  10,   10, 11, 8,  // Left Face
        12, 13, 14,   14, 15, 12, // Right Face
        16, 17, 18,   18, 19, 16, // Top Face
        20, 21, 22,   22, 23, 20  // Bottom Face
    };
}
