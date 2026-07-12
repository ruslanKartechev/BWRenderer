#include "SceneDefinition.h"
#include "Handle.h"
#include "Material.h"
#include "GraphicsGL.h"
#include "Uniforms.h"
#include "Engine.h"

static Handle h_materialFloor;
static Handle h_materialTable;
static Handle h_materialMetal;
constexpr f32 LightDebugScale = .25f;



/// MUST BE USED AFTER SHADERS HAVE BEEN INITIALIZED!
void InitSceneMaterials(AssetManager& assets) {

    // Floor
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_materialFloor);
        material.shaderName = "Default3D";
        material.SetVectorDefinition(ID_COLOR_TINT, {0.5f, 0.5f, 0.5f, 1.0});
        material.SetFloatDefinition(ID_SPECULAR_POWER, 10);
        material.SetTextureDefinition(ID_BASE_MAP, "");
        GL_InitMaterialProperties(material, assets);
    }
    // Table
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_materialTable);
        material.shaderName = "Default3D";
        material.SetVectorDefinition(ID_COLOR_TINT, {0.75f, 0.7f, 0.95f, 1.0});
        material.SetFloatDefinition(ID_SPECULAR_POWER, 15);
        material.SetTextureDefinition(ID_BASE_MAP, "PicnicTable_MTL_baseColor.png");
        GL_InitMaterialProperties(material, assets);
    }
}



void PlaceObjectsToScene(AssetManager& assets, GameScene& scene) {
    // cube RR
    {
        vec3 pos = {4.0f, 0.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube RR", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // cube RL
    {
        vec3 pos = {-4.0f, 0.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube RL", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // cube FR
    {
        vec3 pos = {4.0f, 0.5f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube FR", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // cube FL
    {
        vec3 pos = {-4.0f, 1.0f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube FL", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }

    // Sphere RR
    {
        vec3 pos = {-4.0f, 1.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere RL
    {
        vec3 pos = {4.0f, 1.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere FR
    {
        vec3 pos = {-4.0f, 2.5f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere FL
    {
        vec3 pos = {4.0f, 2.5f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }

    // Capsule
    {
        vec3 pos = {10.0f, 1.0f, -5.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_Capsule(pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Pyramid
    {
        vec3 pos = {0.0f, 2.5f, 0.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_Pyramid(pos, rot, scale, assets.materialDefault3d);
        scene.existingObjects.push_back(objHandle);
    }
    // Table
    {
        vec3 pos = {0.0f, 1.0f, 0.0f};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {0.006f, 0.006f, 0.006f};
        Handle objHandle = scene.CreateObjectWithCustomMesh("upd_picnic table.fbx", pos, rot, scale, h_materialTable);
        scene.existingObjects.push_back(objHandle);
    }
    //Floor
    {
        vec3 pos = {0.0f, -0.1f, 0.0f};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {25.0f, 25.0f, 0.0f};
        Handle objHandle = scene.NewObject_PlaneNamed("FLOOR", pos, rot, scale, h_materialFloor);
        scene.existingObjects.push_back(objHandle);
    }
}

void LoadDefaultTextures(AssetManager& assets) {
    {
        Handle h = {};
        Texture& texture = assets.GetNewTextureObject(h);
        AssetManager::LoadTextureAtPath(texture, "back1.jpg", true);
    }
    {
        Handle h = {};
        Texture& texture = assets.GetNewTextureObject(h);
        AssetManager::LoadTextureAtPath(texture, "PicnicTable_MTL_baseColor.png", true);
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
    light.intensity = 1.2f;
    SET_VEC3(light.color, 1.0f, 1.0f, 1.0f);

    vec3 pos = {0.0f, 4.0f, -4.0f};
    vec3 eulers = {45.0f, 0.0f, 0.0f};
    vec3 scale = {LightDebugScale, LightDebugScale, LightDebugScale};
    Transform_Init(lightTransform);
    Transform_SetLocalPositionRotationScale(lightTransform, pos, eulers, scale);
}


void LoadTextures(AssetManager& assets) {
    {
        Handle h;
        auto& textureOBj = assets.textures.GetNewObjectAndHandle(h);
        AssetManager::LoadTextureAtPath(textureOBj, "PicnicTable_MTL_baseColor.png", true);
    }
    {
        Handle h;
        auto& textureOBj = assets.textures.GetNewObjectAndHandle(h);
        AssetManager::LoadTextureAtPath(textureOBj, "PicnicTable_MTL_normal.png", true);
    }

}

void RunGameScene() {
    printf("Init basic scene\n");
    auto* engine = Engine::GetInstance();

    LoadTextures(engine->assetManager);
    InitSceneMaterials(engine->assetManager);
    PlaceCamera(engine->scene);
    InitSceneLights(engine->scene);

    PlaceObjectsToScene(engine->assetManager, engine->scene);
}
