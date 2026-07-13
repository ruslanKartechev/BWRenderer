#include "RenderObject.h"


void RenderObject::SetName(const char* name)
{
    this->name = name;
}

void RenderObject::SetMaterialAll(const Handle& handle) {
    for (auto& submesh : subMeshses) {
        submesh.hMaterial = handle;
    }
}


void RenderObject::AppendNewMeshAndShader(const Handle& meshHandle, const Handle& shaderHandle) {
    auto size = subMeshses.size();
    subMeshses.resize(size + 1);
    subMeshses[size].hMesh = meshHandle;
    subMeshses[size].hMaterial = shaderHandle;
}
