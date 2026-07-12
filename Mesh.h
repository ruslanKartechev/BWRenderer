#pragma once
#include "SlotsMap.h"
#include "glad/glad.h"


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


void Mesh_Print(Mesh& mesh);

void Mesh_DefaultSphere(Mesh& mesh);
void Mesh_DefaultCapsule(Mesh& mesh);
void Mesh_DefaultPlane(Mesh& mesh);
void Mesh_DefaultQuad(Mesh& mesh);
void Mesh_DefaultDonut(Mesh& mesh);
void Mesh_DefaultPyramid(Mesh& mesh);
void Mesh_DefaultCube(Mesh& mesh);
void ClearMesh(Mesh& mesh);

void Mesh_DefaultCutCone(Mesh& mesh);
