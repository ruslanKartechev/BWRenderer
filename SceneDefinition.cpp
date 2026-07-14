#include "SceneDefinition.h"
#include "Handle.h"
#include "Material.h"
#include "GraphicsGL.h"
#include "Uniforms.h"
#include "Engine.h"
#include <iostream>


constexpr f32 LightDebugScale = .25f;
static Handle h_mat_Floor;
static Handle h_mat_Table;
static Handle h_mat_Metal;
static Handle h_mat_tree;



void LoadDefaultTextures(AssetManager& assets) {
    {
        Handle h = {};
        Texture& texture = assets.GetNewTextureObject(h);
        AssetManager::LoadTextureAtPath(texture, "back1.jpg", true);
    }
}


/// MUST BE USED AFTER SHADERS HAVE BEEN INITIALIZED!
void InitSceneMaterials(AssetManager& assets) {
    const char* defaultShaderName = "Default3D";
    // Floor
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Floor);
        GL_InitMaterialParametersDefault3D(material);
        material.SetVectorDefinition(ID_COLOR_TINT, {0.99f, 0.99f, 0.99f, 1.0});
        material.SetFloatDefinition(ID_SMOOTHNESS, .6f);
        material.SetFloatDefinition(ID_METALLIC, .55f);

        material.SetTextureDefinition(ID_BASE_MAP, "tile_concrete.png");
        material.SetTextureDefinition(ID_NORMAL_MAP, "tile_concrete_normal.jpg");
        material.SetVectorDefinition(ID_BASE_MAP_TO, {8.0f, 8.0f, 0.0f, 0.0f});

        assets.LoadTexturesForMaterials(material);
        GL_InitMaterialProperties(material, assets);
    }
    // Table
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Table);
        GL_InitMaterialParametersDefault3D(material);
        material.SetVectorDefinition(ID_COLOR_TINT, {0.75f, 0.7f, 0.95f, 1.0});
        material.SetFloatDefinition(ID_SMOOTHNESS, .25f);
        material.SetFloatDefinition(ID_METALLIC, .25f);

        material.SetTextureDefinition(ID_BASE_MAP, "PicnicTable_MTL_baseColor.png");
        material.SetTextureDefinition(ID_NORMAL_MAP, "PicnicTable_MTL_normal.png");
        assets.LoadTexturesForMaterials(material);
        GL_InitMaterialProperties(material, assets);
    }
    // Metallic
    {
        std::cout << "Metallic material init" << std::endl;
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Metal);
        GL_InitMaterialParametersDefault3D(material);

        material.SetFloatDefinition(ID_SMOOTHNESS, 0.65f);
        material.SetFloatDefinition(ID_METALLIC, 1.0f);

        constexpr float shade = .05f;
        material.SetVectorDefinition(ID_COLOR_TINT, {shade, shade, shade, 1.0});

        material.SetVectorDefinition(ID_BASE_MAP_TO, {1.0f, 1.0f, 0.0f, 0.0f});
        // assets.LoadTexturesForMaterials(material);
        GL_InitMaterialProperties(material, assets);
    }
    // Tree mat 1
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_tree);
        material.shaderName = "Tree";

        material.SetFloatDefinition(ID_ALPHA_CLIP_VALUE, .5f);
        material.SetVectorDefinition(ID_COLOR_TINT, {0.99f, 0.99f, 0.99f, 1.0});
        material.SetTextureDefinition(ID_BASE_MAP, "TAI_Atlas_1A.tga");

        bool didLoad = assets.LoadTexturesForMaterials(material);
        if (didLoad == false) {
            material.SetTextureDefinition(ID_BASE_MAP, ""); // default
        }

        GL_InitMaterialProperties(material, assets);
    }
}

void LoadMeshes(AssetManager& assets) {


}



void PlaceObjectsToScene(AssetManager& assets, GameScene& scene) {
    const float R = 4;
    const float L = -4;
    const float F = -5;
    const float FStep = 4;
    float posZ = F;

    // cube RR
    {
        vec3 pos = {R, 0.5f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Cube RR", assets.meshCube ,pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // cube RL
    {
        vec3 pos = {L, 0.5f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Cube RL", assets.meshCube, pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Pyramid RL
    {
        vec3 pos = {L, 1.85f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};

        Handle objHandle = scene.NewObject_SingleSubMesh("", assets.meshPyramid, pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Pyramid RR
    {
        vec3 pos = {R, 1.85f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};

        Handle objHandle = scene.NewObject_SingleSubMesh("", assets.meshPyramid, pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }

    posZ += FStep;
    // cube FR
    {
        vec3 pos = {R, 0.5f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Cube FR", assets.meshCube, pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // cube FL
    {
        vec3 pos = {L, 1.0f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Cube FL", assets.meshCube, pos, rot, scale, h_mat_Metal);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere FR
    {
        vec3 pos = {L, 2.5f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Sphere", assets.meshSphere, pos, rot, scale, h_mat_Metal);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere FL
    {
        vec3 pos = {R, 2.5f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Sphere", assets.meshSphere, pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }

    // Capsule
    {
        vec3 pos = {0.0f, 1.0f, -3.5f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("capsule", assets.meshCapsule, pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Table
    {
        vec3 pos = {0.0f, 0.65f, 0.0f};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {0.01f, 0.01f, 0.01f};
        std::vector<Handle> newHandles = {};
        assets.LoadModelsFromFbx("upd_picnic table.fbx", scene, newHandles);

        if (newHandles.empty()) {
            std::cerr << "Failed to load any objects!" << std::endl;
            return ;
        }

        auto& mainRO = scene.renderObjects.GetItemRef(newHandles[0]);
        scene.AddCustomObject(newHandles[0], pos, rot, scale);
        mainRO.subMeshses[0].hMaterial = h_mat_Table;
    }
    posZ += FStep;
    // Tree 1
    {
        vec3 pos = {R, 0.0f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        std::vector<Handle> newHandles = {};
        assets.LoadModelsFromFbx("TAI_Tree_03A.fbx", scene, newHandles);

        std::cout << "[SceneDef] TREE Loaded objects count: " << newHandles.size() << std::endl;
        if (newHandles.empty()) {
            std::cerr << "Failed to load any objects!" << std::endl;
            return ;
        }
        auto objIdx = 0;
        // auto objIdx = newHandles.size() - 1;
        // objIdx = 1;
        auto& mainRO = scene.renderObjects.GetItemRef(newHandles[objIdx]);
        mainRO.subMeshses[0].hMaterial = h_mat_tree;
        scene.AddCustomObject(newHandles[objIdx], pos, rot, scale);
    }
    {
        vec3 pos = {L, 0.0f, posZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        std::vector<Handle> newHandles = {};
        assets.LoadModelsFromFbx("TAI_Tree_03A.fbx", scene, newHandles);

        auto objIdx = 0;
        auto& mainRO = scene.renderObjects.GetItemRef(newHandles[objIdx]);
        mainRO.subMeshses[0].hMaterial = h_mat_tree;
        scene.AddCustomObject(newHandles[objIdx], pos, rot, scale);
    }

    //Floor
    {
        vec3 pos = {0.0f, -0.01f, 0.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {32.0f, 1.0f, 32.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("FloorPlane", assets.meshPlane, pos, rot, scale, h_mat_Floor);
        scene.existingObjects.push_back(objHandle);
    }
}


void PlaceCamera(GameScene& scene) {
    Transform& cameraTransform = scene.transforms.GetNewObjectAndHandle(scene.camera.transformHandle);
    Transform_SetWorldPosition(cameraTransform, 0.0f, 1.0f, -10.0f);
    Transform_SetRotationEulerDeg(cameraTransform, 1.0f, 0.0f, 0.0f);
    Transform_SetLocalScale(cameraTransform, 1.0f, 1.0f, 1.0f);
}


void InitSceneLights(GameScene& scene){
    Light& light = scene.mainLight;
    Transform& lightTransform = scene.transforms.GetNewObjectAndHandle(light.transformHandle);
    light.lightType = ELightType::Directional;
    light.intensity = 0.5f;

    SET_VEC3(light.color, 1.0f, 1.0f, 1.0f);
    scene.ambientIntensity = .05f;

    const float bright = .8f;
    SET_VEC3(scene.ambientLightColor, bright, bright, bright);

    vec3 pos = {0.0f, 10.0f, -20.0f};
    vec3 eulers = {0.0f, 0.0f, 0.0f};
    vec3 scale = {LightDebugScale, LightDebugScale, LightDebugScale};
    Transform_Init(lightTransform);
    Transform_SetLocalPositionRotationScale(lightTransform, pos, eulers, scale);
    Transform_RotateWorldY(lightTransform, 30.0);
    Transform_RotateLocalX(lightTransform, 50.0);
}

void RunGameSceneInit() {
    auto* engine = Engine::GetInstance();

    InitSceneMaterials(engine->assetManager);
    PlaceCamera(engine->scene);
    InitSceneLights(engine->scene);

    PlaceObjectsToScene(engine->assetManager, engine->scene);
}
