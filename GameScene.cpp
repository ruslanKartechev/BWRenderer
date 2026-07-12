#include "GameScene.h"
#include "Mesh.h"
#include "GraphicsGL.h"



Transform& GameScene::GetTransformForObject(Handle& renderObjectHandle) {
    RenderObject& obj = renderObjects.GetItemRef(renderObjectHandle);
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

    Handle handleRenderObj = renderObjects.GetFreeHandle();
    Handle handleTransform = transforms.GetFreeHandle();
    Transform &tr = transforms.GetItemRef(handleTransform);
    RenderObject &obj = renderObjects.GetItemRef(handleRenderObj);

    Transform_Init(tr);
    obj.hTransform = handleTransform;
    obj.AppendNewMeshAndShader(hMesh, hMaterial);
    GL_AllocateGraphicsForObject(obj, *this);

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);
    obj.SetName(name);
    return handleRenderObj;
}

Handle GameScene::CreateObjectWithCustomMesh(const char* path, vec3 position, vec3 rotation, vec3 scale, Handle materialHandle) {
    Handle objHandle = AssetManager::LoadModel(path, *this, 1);

    RenderObject& obj = renderObjects.GetItemRef(objHandle);
    Transform& tr = transforms.GetItemRef(obj.hTransform);

    obj.shadersAssigned = true;
    for (auto& subMesh : obj.meshData) {
        subMesh.hMaterial = materialHandle;
    }
    // obj.AppendNewMeshAndShader(handleMesh, shaderHandle);
    GL_AllocateGraphicsForObject(obj, *this);

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);
    obj.SetName(path);

    // Mesh& mesh = scene.meshes.GetItemRef(ro.meshData[0].hMesh);
    // Mesh_Print(mesh);
    return objHandle;
}



Handle GameScene::NewObject_CubeNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCube(mesh);
    return NewObject_SingleSubMesh(name, handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_Cube(vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCube(mesh);
    return NewObject_SingleSubMesh("Cube", handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_SphereNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultSphere(mesh);
    return NewObject_SingleSubMesh(name, handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_Sphere(vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultSphere(mesh);
    return NewObject_SingleSubMesh("Sphere", handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_PyramidNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPyramid(mesh);
    return NewObject_SingleSubMesh(name, handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_Pyramid(vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPyramid(mesh);
    return NewObject_SingleSubMesh("Pyramid", handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_CapsuleNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCapsule(mesh);
    return NewObject_SingleSubMesh(name, handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_Capsule(vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCapsule(mesh);
    return NewObject_SingleSubMesh("Capsule", handleMesh, position, rotation, scale, hMaterial);
}

Handle GameScene::NewObject_PlaneNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {
    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPlane(mesh);
    return NewObject_SingleSubMesh(name, handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_Plane(vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPlane(mesh);
    return NewObject_SingleSubMesh("Plane", handleMesh, position, rotation, scale, hMaterial);
}

Handle GameScene::NewObject_CutConeNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {
    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCutCone(mesh);
    return NewObject_SingleSubMesh(name, handleMesh, position, rotation, scale, hMaterial);
}
Handle GameScene::NewObject_CutCone(vec3 position, vec3 rotation, vec3 scale, Handle hMaterial) {
    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCutCone(mesh);
    return NewObject_SingleSubMesh("CutCone", handleMesh, position, rotation, scale, hMaterial);
}



