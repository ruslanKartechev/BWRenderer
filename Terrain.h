#pragma once
#include "MyTypes.h"
#include "Mesh.h"
#include "RenderMode.h"
#include "RenderObject.h"
#include "Handle.h"

class Terrain {
public:
    Handle hMaterial = {};
    Handle hTransform = {};
    float width;
    float height;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    void GenerateMeshData();
    void FreeData();
    float* GetVertexDataPtr();
    int* GetIndexDataPtr();

    int vertexDataCount;
    int indexCount;
    int stride;
    int startIndexVertex = -1;
    int startIndexUV = -1;
    int startIndexColor = -1;
    int startIndexNormals = -1;
    int startIndexTangent = -1;
    int cellsCountX;
    int cellsCountY;

    bool isBuilt;

private:
    float* vertexData = nullptr;
    int* indexData = nullptr;

};

