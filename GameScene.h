#pragma once
#include "Transform.h"
#include "Light.h"
#include "RenderObject.h"
#include "Skybox.h"
#include "Camera.h"
#include "Terrain.h"
#include "UIObject.h"


class GameScene {

public:
    std::vector<Handle> activeWorldHandles = {};
    std::vector<Handle> activeUIHandles = {};

    SlotMap<Transform> transforms = SlotMap<Transform>();
    SlotMap<RenderObject> worldObjectsPool = SlotMap<RenderObject>();
    SlotMap<UIObject> uiObjectsPool = SlotMap<UIObject>();

    Camera camera = {};
    Light mainLight = {};
    Skybox skybox = {};
    Terrain terrain = {};


    vec4 backgroundColor = {0.8f, 0.5f, 0.76f, 1.0f};
    vec3 ambientLightColor = {1.0f, 0.9f, 0.9f};
    float ambientIntensity = 0.5f;

    Transform& GetTransformForObject(ObjectHandle& objectHandle);


    /**
     * Creates a new RenderObject and assigns a renderSubMesh with a given mesh and shader
     * @param name Name of the object
     * @param hMesh Handle for existing mesh
     * @param shaderHandle Handle to the shader used
     * @return handle to the new RenderObject
     */
    ObjectHandle NewObject_SingleSubMesh(const char* name, MeshHandle hMesh, vec3 position, vec3 rotation, vec3 scale, ShaderHandle shaderHandle);

    Transform& GetCameraTransform();
};
