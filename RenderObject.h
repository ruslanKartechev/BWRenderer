#pragma once
#include <iostream>
#include <glad/glad.h>
#include "Transform.h"
#include "DataStructures.h"
#include "Mesh.h"
#include "Slot.h"

class RenderObject {
public:
    std::vector<MeshRenderData> meshData;
    Handle hTransform;
    std::string name;
    bool shadersAssigned;

    void SetName(const char* name);

    void AppendNewMeshAndShader(const Handle meshHandle, const Handle shaderHandle);

};

