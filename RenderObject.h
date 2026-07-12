#pragma once
#include "RenderSubMesh.h"
#include "SlotsMap.h"
#include <string>

class RenderObject {
public:
    std::vector<RenderSubMesh> meshData;
    Handle hTransform;
    std::string name;
    bool shadersAssigned;

    void SetName(const char* name);

    void AppendNewMeshAndShader(const Handle meshHandle, const Handle shaderHandle);

};

