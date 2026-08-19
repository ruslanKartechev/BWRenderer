#include "GameScene.h"

#include "Engine.h"
#include "GraphicsGL.h"


Transform& GameScene::GetCameraTransform() {
    return transforms.GetItemRef(camera.transformHandle);
}

Transform& GameScene::GetTransformForObject(Handle& objectHandle) {
    RenderObject& obj = worldObjectsPool.GetItemRef(objectHandle);
    return transforms.GetItemRef(obj.hTransform);
}

/**
 * Creates a new RenderObject and assigns a renderSubMesh with a given mesh and shader
 * @param name Name of the object
 * @param hMesh Handle for existing mesh
 * @param hMaterial Handle to the material used
 * @return handle to the new RenderObject
 */
Handle GameScene::NewObject_SingleSubMesh(const char *name,
    Handle hMesh,
    vec3 position,
    vec3 rotation,
    vec3 scale,
    Handle hMaterial)
{

    Handle handleRenderObj = worldObjectsPool.GetFreeHandle();
    Handle handleTransform = transforms.GetFreeHandle();
    Transform &tr = transforms.GetItemRef(handleTransform);
    RenderObject &obj = worldObjectsPool.GetItemRef(handleRenderObj);

    Transform_Init(tr);
    obj.hTransform = handleTransform;
    obj.AppendNewMeshAndShader(hMesh, hMaterial);
    GL_AllocateGraphicsForObject(obj, Engine::GetInstance()->assetManager);

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);
    obj.SetName(name);
    return handleRenderObj;
}


