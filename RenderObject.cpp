#include "RenderObject.h"


void RenderObject::SetName(const char* name)
{
    this->name = name;
}


void RenderObject::AppendNewMeshAndShader(const Handle meshHandle, const Handle shaderHandle) {
    auto size = meshData.size();
    meshData.resize(size + 1);
    meshData[size].hMesh = meshHandle;
    meshData[size].hShader = shaderHandle;
}
