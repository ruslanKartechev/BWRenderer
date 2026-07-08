
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
 * @param handleMesh Handle for existing mesh
 * @param shaderHandle Handle to the shader used
 * @return handle to the new RenderObject
 */
Handle GameScene::NewObject_MeshShader(const char *name, Handle handleMesh, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {
    Handle handleRenderObj = renderObjects.GetFreeHandle();
    Handle handleTransform = transforms.GetFreeHandle();
    Transform &tr = transforms.GetItemRef(handleTransform);
    RenderObject &obj = renderObjects.GetItemRef(handleRenderObj);

    printf("---%s GOT transform handle %d, %d\n\n", name, handleTransform.index, handleTransform.generation);
    printf("--- Transform ptr %p\n", &tr);

    Transform_Init(tr);
    obj.hTransform = handleTransform;
    obj.AppendNewMeshAndShader(handleMesh, shaderHandle);
    AllocateGraphicsForObject(obj, *this);

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);
    obj.SetName(name);
    return handleRenderObj;
}


Handle GameScene::NewObject_CubeNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCube(mesh);
    return NewObject_MeshShader(name, handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_Cube(vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCube(mesh);
    return NewObject_MeshShader("Cube", handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_SphereNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultSphere(mesh);
    return NewObject_MeshShader(name, handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_Sphere(vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultSphere(mesh);
    return NewObject_MeshShader("Sphere", handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_PyramidNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPyramid(mesh);
    return NewObject_MeshShader(name, handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_Pyramid(vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPyramid(mesh);
    return NewObject_MeshShader("Pyramid", handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_CapsuleNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCapsule(mesh);
    return NewObject_MeshShader(name, handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_Capsule(vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCapsule(mesh);
    return NewObject_MeshShader("Capsule", handleMesh, position, rotation, scale, shaderHandle);
}

Handle GameScene::NewObject_PlaneNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {
    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPlane(mesh);
    return NewObject_MeshShader(name, handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_Plane(vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {

    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultPlane(mesh);
    return NewObject_MeshShader("Plane", handleMesh, position, rotation, scale, shaderHandle);
}

Handle GameScene::NewObject_CutConeNamed(const char* name, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {
    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCutCone(mesh);
    return NewObject_MeshShader(name, handleMesh, position, rotation, scale, shaderHandle);
}
Handle GameScene::NewObject_CutCone(vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {
    Handle handleMesh = meshes.GetFreeHandle();
    Mesh& mesh = meshes.GetItemRef(handleMesh);
    Mesh_DefaultCutCone(mesh);
    return NewObject_MeshShader("CutCone", handleMesh, position, rotation, scale, shaderHandle);
}