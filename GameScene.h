#pragma once
#include "Transform.h"
#include "DataStructures.h"
#include "Mesh.h"
#include "RenderObject.h"
#include "Skybox.h"
#include "Camera.h"


class GameScene {

    public:
    std::vector<Handle> existingObjects = {};

    SlotMap<Transform> transforms = SlotMap<Transform>();
    SlotMap<RenderObject> renderObjects = SlotMap<RenderObject>();

    Camera camera = {};
    Light mainLight = {};
    Skybox skybox = {};

    vec4 backgroundColor = {0.8f, 0.5f, 0.76f, 1.0f};
    vec3 ambientLightColor = {1.0f, 0.9f, 0.9f};
    float ambientIntensity = 0.5f;

    Transform& GetTransformForObject(Handle& renderObjectHandle);


    /**
     * Creates a new RenderObject and assigns a renderSubMesh with a given mesh and shader
     * @param name Name of the object
     * @param hMesh Handle for existing mesh
     * @param shaderHandle Handle to the shader used
     * @return handle to the new RenderObject
     */
    Handle NewObject_SingleSubMesh(const char* name, Handle hMesh, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle);

};
