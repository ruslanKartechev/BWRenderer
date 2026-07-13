#pragma once
#include "RenderSubMesh.h"
#include "SlotsMap.h"
#include <string>

class RenderObject {
public:
    std::vector<RenderSubMesh> subMeshses;
    Handle hTransform;
    std::string name;

    void SetName(const char* name);

    void AppendNewMeshAndShader(const Handle& meshHandle, const Handle& shaderHandle);

    void SetMaterialAll(const Handle& handle);
};

