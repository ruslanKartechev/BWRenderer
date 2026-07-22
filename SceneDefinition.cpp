#include "SceneDefinition.h"
#include "Handle.h"
#include "Material.h"
#include "GraphicsGL.h"
#include "Uniforms.h"
#include "Engine.h"
#include <iostream>

#include "InstancedBuffer.h"
#include "NoiseGenerator.h"


constexpr f32 LightDebugScale = .25f;

static MaterialHandle h_mat_Floor;
static MaterialHandle h_mat_Table;
static MaterialHandle h_mat_Metal;
static MaterialHandle h_mat_Tree;
static MaterialHandle h_mat_Terrain;

static MeshHandle hMeshTable = {};
static MeshHandle hMeshTree = {};
static MeshHandle hMeshLamp = {};

static std::shared_ptr<InstancedBuffer> treesInstanceBuffer = std::make_shared<InstancedBuffer>();


void LoadDefaultTextures(AssetManager& assets) {
    {
        Handle h = {};
        Texture& texture = assets.GetNewTextureObject(h);
        AssetManager::LoadTextureAtPath(texture, "back1.jpg", true);
    }
}


void LoadMaterials(AssetManager& assets) {
    const char* defaultShaderName = "Default3D";
    // Floor
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Floor);
        GL_InitMaterialParametersDefault3D(material);
        constexpr float c = 0.25f;
        material.SetVectorDefinition(ID_COLOR_TINT, {c,c,c, 1.0});
        material.SetFloatDefinition(ID_SMOOTHNESS, .5f);
        material.SetFloatDefinition(ID_METALLIC, .25f);
        material.SetFloatDefinition(ID_SSR_POWER, 1.0f);

        material.SetTextureDefinition(ID_BASE_MAP, "tile_concrete.png", 0);
        material.SetTextureDefinition(ID_NORMAL_MAP, "tile_concrete_normal.jpg", 1);
        material.SetVectorDefinition(ID_BASE_MAP_TO, {8.0f, 8.0f, 0.0f, 0.0f});

        assets.LoadTexturesForMaterials(material);
        GL_InitMaterialProperties(material, assets);
    }
    // Table
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Table);
        GL_InitMaterialParametersDefault3D(material);
        material.SetVectorDefinition(ID_COLOR_TINT, {0.75f, 0.45f, 0.45f, 1.0});
        material.SetFloatDefinition(ID_SMOOTHNESS, .25f);
        material.SetFloatDefinition(ID_METALLIC, .25f);

        material.SetTextureDefinition(ID_BASE_MAP, "PicnicTable_MTL_baseColor.png", 0);
        material.SetTextureDefinition(ID_NORMAL_MAP, "PicnicTable_MTL_normal.png", 1);
        assets.LoadTexturesForMaterials(material);
        GL_InitMaterialProperties(material, assets);
    }
    // Metallic
    {
        std::cout << "Metallic material init" << std::endl;
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Metal);
        GL_InitMaterialParametersDefault3D(material);

        material.SetFloatDefinition(ID_SMOOTHNESS, 0.8f);
        material.SetFloatDefinition(ID_METALLIC, 1.0f);

        constexpr float shade = .25f;
        material.SetVectorDefinition(ID_COLOR_TINT, {shade, shade, shade, 1.0});
        material.SetVectorDefinition(ID_BASE_MAP_TO, {1.0f, 1.0f, 0.0f, 0.0f});
        GL_InitMaterialProperties(material, assets);
    }
    // Tree mat 1
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Tree);
        material.shaderName = "Tree";

        material.SetFloatDefinition(ID_ALPHA_CLIP_VALUE, .5f);
        material.SetVectorDefinition(ID_COLOR_TINT, {0.99f, 0.99f, 0.99f, 1.0});
        material.SetTextureDefinition(ID_BASE_MAP, "TAI_Atlas_1A.tga", 0);

        bool didLoad = assets.LoadTexturesForMaterials(material);
        if (didLoad == false) {
            material.SetTextureDefinition(ID_BASE_MAP, "", 0); // default
        }

        GL_InitMaterialProperties(material, assets);
    }
    // Terrain
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Terrain);
        material.shaderName = "Terrain";
        material.SetFloatDefinition(ID_SMOOTHNESS, 0.4f);
        constexpr float shade = .5f;
        material.SetVectorDefinition(ID_COLOR_TINT, {shade, shade, shade, 1.0});
        GL_InitMaterialProperties(material, assets);
    }
}


void LoadMeshes(AssetManager& assets) {
    std::vector<ObjectDefinition> definitions = {};

    // table
    {
        definitions.clear();
        assets.LoadDefinitions("picnic_table.fbx", definitions);
        hMeshTable = definitions[0].Meshes[0];
    }
    // Tree
    {
        definitions.clear();
        assets.LoadDefinitions("TAI_Tree_03A.fbx", definitions);
        hMeshTree = definitions[0].Meshes[0];
    }
    // Lamp
    {
        definitions.clear();
        assets.LoadDefinitions("upd_lamp.fbx", definitions);
        hMeshLamp = definitions[0].Meshes[0];
    }

    // std::vector<std::string> paths = {
    //     "upd_picnic table.fbx",
    //     "upd_lamp.fbx",
    //     "TAI_Tree_03A.fbx"
    // };
    // loadedDefinitions.reserve(12);
    // for (auto& path : paths) {
    //     std::vector<ObjectDefinition> definitions = {};
    //     std::cout << "[INIT] Loading: " << path << std::endl;
    //     assets.LoadDefinitions(path.c_str(), definitions);
    //     std::cout << "Found " << definitions.size() << " object definitions" << std::endl;
    //
    //     int i = 1;
    //     for (auto& def : definitions) {
    //         std::cout << "Object "<<i++<<" Meshes count: "<< def.Meshes.size() << std::endl;
    //     }
    //     loadedDefinitions.insert(loadedDefinitions.end(), definitions.begin(), definitions.end());
    // }
    // for (auto& def : loadedDefinitions) {
    //     std::cout << "[Init] definition: " << def.name << " MeshesCount: " << def.Meshes.size() << std::endl;
    // }
}


void AddNewCustomObject_SingleMesh(std::string&& name, AssetManager& assets, GameScene& scene,
                    Handle hMesh, Handle hMaterial,
                    vec3 pos, vec3 rot, vec3 scale) {

    Handle hObject = scene.NewObject_SingleSubMesh(name.c_str(), hMesh, pos, rot, scale, hMaterial);
    scene.activeWorldHandles.push_back(hObject);
}


void PlaceTrees(AssetManager& assets, GameScene& scene) {
    float posTreeZ = 5;
    // Tree 1
    {
        vec3 pos = {-3.0, 0.0f, posTreeZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Tree 1", assets, scene, hMeshTree, h_mat_Tree, pos, rot, scale);
    }
    // Tree 2
    {
        vec3 pos = {0.0, 0.0f, posTreeZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Tree 2", assets, scene, hMeshTree, h_mat_Tree, pos, rot, scale);
    }
    // Tree 2
    {
        vec3 pos = {3.0, 0.0f, posTreeZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Tree 3", assets, scene, hMeshTree, h_mat_Tree, pos, rot, scale);
    }

}

void PlaceDefaultShapes(AssetManager& assets, GameScene& scene) {
    float posGeomZ = -2.5f;
    float posGeomX = -7.0f;
    float posGeomXStep = 2.5f;
   // cube 1
    {
        vec3 pos = {posGeomX, 1.0f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Cube FL", assets.meshCube, pos, rot, scale, h_mat_Metal);
        scene.activeWorldHandles.push_back(objHandle);
    }
    posGeomX += posGeomXStep;
    // cube 2
    {
        vec3 pos = {posGeomX, 0.5f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Cube RR", assets.meshCube ,pos, rot, scale, assets.materialDefault3d);
        scene.activeWorldHandles.push_back(objHandle);
    }
    posGeomX += posGeomXStep;
    // Pyramid
    {
        vec3 pos = {posGeomX, .5f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Pyramid", assets.meshPyramid, pos, rot, scale, assets.materialDefault3d);
        scene.activeWorldHandles.push_back(objHandle);
    }
    posGeomX += posGeomXStep;
    // Sphere 1
    {
        vec3 pos = {posGeomX, .5f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Sphere", assets.meshSphere, pos, rot, scale, h_mat_Metal);
        scene.activeWorldHandles.push_back(objHandle);
    }
    posGeomX += posGeomXStep;
    // Sphere 2
    {
        vec3 pos = {posGeomX, .5f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("Sphere", assets.meshSphere, pos, rot, scale, assets.materialDefault3d);
        scene.activeWorldHandles.push_back(objHandle);
    }
    posGeomX += posGeomXStep;
    // Capsule
    {
        vec3 pos = {posGeomX, 1.0f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("capsule", assets.meshCapsule, pos, rot, scale, assets.materialDefault3d);
        scene.activeWorldHandles.push_back(objHandle);
    }
    posGeomX += posGeomXStep;
    // Donut
    {
        vec3 pos = {posGeomX, 1.0f, posGeomZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("donut", assets.meshDonut, pos, rot, scale, h_mat_Metal);
        scene.activeWorldHandles.push_back(objHandle);
    }

}

void PlaceCustomShapes(AssetManager& assets, GameScene& scene) {
    float posTableZ = 2;
    float posLampsZ = 10;

    // Table 1
    {
        vec3 pos = {-3.5, 0.6f, posTableZ};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {0.0065f, 0.0065f, 0.0065f};
        AddNewCustomObject_SingleMesh("Table 1", assets, scene, hMeshTable, h_mat_Table, pos, rot, scale);
    }
    // Table 2
    {
        vec3 pos = {3.5, 0.6f, posTableZ};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {0.0065f, 0.0065f, 0.0065f};
        AddNewCustomObject_SingleMesh("Table 2", assets, scene, hMeshTable, h_mat_Table, pos, rot, scale);
    }

    // Lamp 1
    {
        vec3 pos = {-3.0, 3.0f, posLampsZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Lamp 1", assets, scene, hMeshLamp, h_mat_Metal, pos, rot, scale);
    }
    // Lamp 2
    {
        vec3 pos = {0.0, 3.0f, posLampsZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Lamp 2", assets, scene, hMeshLamp, h_mat_Metal, pos, rot, scale);
    }
    // Lamp 2
    {
        vec3 pos = {3.0, 3.0f, posLampsZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Lamp 3", assets, scene, hMeshLamp, h_mat_Metal, pos, rot, scale);
    }
}



void BuildTerrain(AssetManager& assets, GameScene& scene) {

    std::cout << "BUILDING TERRAIN" << std::endl;
    Terrain& terr = scene.terrain;

    terr.hMaterial = h_mat_Terrain;
    Transform& transform = scene.transforms.GetNewObjectAndHandle(terr.hTransform);
    Transform_Init(transform);
    SET_VEC3(transform.position, 0.0f, -0.1f, 0.0f);
    Transform_SetRotationEulerDeg(transform, 0.0, 0.0, 0.0);
    Transform_UpdateMatrices(transform);

    terr.GenerateMeshData();
    GL_AllocateGraphicsTerrain(terr);
}


void PlaceObjectsToScene(AssetManager& assets, GameScene& scene) {
    //Floor
    {
        vec3 pos = {0.0, -0.5f, 0.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {32.0f, 1.0f, 32.0f};
        Handle objHandle = scene.NewObject_SingleSubMesh("FloorPlane", assets.meshCube, pos, rot, scale, h_mat_Floor);
        scene.activeWorldHandles.push_back(objHandle);
    }
    // PlaceDefaultShapes(assets, scene);
    PlaceCustomShapes(assets, scene);

    BuildTerrain(assets, scene);
    std::cout << "Building Terrain completed" << std::endl;
}


void PlaceCamera(GameScene& scene) {
    Transform& cameraTransform = scene.transforms.GetNewObjectAndHandle(scene.camera.transformHandle);
    Transform_SetWorldPosition(cameraTransform, 0.0f, 3.5f, -11.0f);
    Transform_SetRotationEulerDeg(cameraTransform, 10.0f, 0.0f, 0.0f);
    Transform_SetLocalScale(cameraTransform, 1.0f, 1.0f, 1.0f);
}



void InitSceneLights(GameScene& scene){
    Light& light = scene.mainLight;
    Transform& lightTransform = scene.transforms.GetNewObjectAndHandle(light.transformHandle);
    light.lightType = ELightType::Directional;
    // light.intensity = 0.5f;
    // SET_VEC3(light.color, 1.0f, 1.0f, 1.0f);
    // scene.ambientIntensity = .05f;
    // const float bright = .8f;
    // SET_VEC3(scene.ambientLightColor, bright, bright, bright);

    vec3 pos = {0.0f, 10.0f, -20.0f};
    vec3 eulers = {0.0f, 0.0f, 0.0f};
    vec3 scale = {LightDebugScale, LightDebugScale, LightDebugScale};
    Transform_Init(lightTransform);
    Transform_SetLocalPositionRotationScale(lightTransform, pos, eulers, scale);
    Transform_RotateWorldY(lightTransform, 30.0);
    Transform_RotateLocalX(lightTransform, 50.0);
}




void InitInstanceBuffers(AssetManager& assets) {
    const i32 count = 100;
    const i32 entriesPerRow = 10;
    const float spacing = 6.0f;
    f32 startX = -1.0f * entriesPerRow * .5f * spacing;
    f32 startY = -1.0f * entriesPerRow * .5f * spacing;

    auto& buffer = *treesInstanceBuffer;
    buffer.name = "Trees";
    buffer.renderData.hMesh = hMeshTree;
    buffer.renderData.hMaterial = h_mat_Tree;
    GL_AllocateGraphicsMesh(buffer.renderData, assets);

    for (size_t i = 0; i < count; i++) {
        i32 x = i % entriesPerRow;
        i32 y = (i / entriesPerRow);
        vec3 position = {startX + x * spacing, 0, startY + y * spacing};
        versor rotation = {0.0f, 0.0f, 0.0f, 1.0f};
        vec3 scale = {1., 1., 1.};
        glm_vec3_scale(scale, 0.01f, scale);

        auto& entry = buffer.AddNewOne();
        Transform_UpdateMatrixOnly(entry.modelMatrix, position, rotation, scale);
    }
    GL_AddInstanceBuffer(treesInstanceBuffer);
}



static Handle h_debugUIQuad;
static TextureHandle h_noiseTex;
static MaterialHandle h_matUI;
static NoiseGenerator noiseGen = {};

constexpr i32 NoiseSizeX = 200;
constexpr i32 NoiseSizeY = 200;

void InitNoiseDebugUI(Engine& engine) {

    std::cout << "Generating Noise" << std::endl;
    noiseGen.sizeX = NoiseSizeX;
    noiseGen.sizeY = NoiseSizeY;
    noiseGen.octaves = 8;
    noiseGen.GeneratePerlin();

    std::cout << "Initializing debug NoiseTextureView" << std::endl;

    UIObject& obj = engine.scene.uiObjectsPool.GetNewObjectAndHandle(h_debugUIQuad);
    Material& uiMat = engine.assetManager.materials.GetNewObjectAndHandle(h_matUI);
    uiMat.shaderName = "UIQuad";
    uiMat.vectorsDefinitions.clear();
    uiMat.floatsDefinitions.clear();
    uiMat.texturesDefinitions.clear();
    obj.hMaterial = h_matUI;

    SET_VEC2(obj.position, 10, 10);
    SET_VEC2(obj.size, NoiseSizeX, NoiseSizeY);

    GL_AllocateGUIQuad(obj);
    engine.scene.activeUIHandles.push_back(h_debugUIQuad);

    f32* noiseTexPtr = noiseGen.GetDataPtr();

    const char* noiseTexName = "Noise_Debug";
    Texture& noiseTexture = engine.assetManager.textures.GetNewObjectAndHandle(h_noiseTex);
    noiseTexture.name = noiseTexName;
    noiseTexture.SetPixelFormat(Texture::TEX_FORMAT_R32);
    noiseTexture.SetSize(noiseGen.sizeX, noiseGen.sizeY);
    noiseTexture.SetChannelCount(1);
    noiseTexture.SetFloatDataPtr(noiseTexPtr);
    noiseTexture.UploadToGL(false);

    uiMat.SetTextureDefinition(ID_MAIN_TEXTURE, noiseTexName, 0);
    GL_InitMaterialProperties(uiMat, engine.assetManager);

}


void RunGameSceneInit() {
    auto& engine = *Engine::GetInstance();

    LoadMeshes(engine.assetManager);
    LoadMaterials(engine.assetManager);
    PlaceCamera(engine.scene);
    InitSceneLights(engine.scene);
    PlaceObjectsToScene(engine.assetManager, engine.scene);


    InitNoiseDebugUI(engine);

    InitInstanceBuffers(engine.assetManager);
}
