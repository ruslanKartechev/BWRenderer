#pragma once
#include "Transform.h"
#include "DataStructures.h"
#include "Mesh.h"
#include "RenderObject.h"

class GameScene {
public:
    std::vector<Handle> existingObjects = {};

    SlotMap<Mesh> meshes = {};
    SlotMap<Transform> transforms = {};
    SlotMap<RenderObject> renderObjects = {};

    Camera camera = {};
    Light mainLight = {};

    vec4 backgroundColor = {0.8f, 0.5f, 0.76f, 1.0f};
    vec3 ambientLightColor = {0.4f, 1.0f, 1.0f};
    float ambientIntensity = 0.2;
};
