#include "ScriptTerrainGame.h"
#include <iostream>

#include "imgui.h"
#include "../Handle.h"
#include "../Material.h"
#include "../GraphicsGL.h"
#include "../Uniforms.h"
#include "../Engine.h"

#include "../InstancedBuffer.h"
#include "../NoiseGenerator.h"


static MaterialHandle h_mat_Floor;
static MaterialHandle h_mat_Table;
static MaterialHandle h_mat_Metal;
static MaterialHandle h_mat_Tree;
static MaterialHandle h_mat_Terrain;

static MeshHandle hMeshTable = {};
static MeshHandle hMeshTree = {};
static MeshHandle hMeshLamp = {};

constexpr f32 LightDebugScale = .25f;

static Handle h_objUIQuad;
static MaterialHandle h_matUI;

const char* noiseTexName = "TerrainHeight";
const char* splatMapTexName = "TerrainSplatMap";

static TextureHandle h_sandTex;
static TextureHandle h_grassTex;
static TextureHandle h_rockTex;

constexpr i32 NoiseSizeX = 256;
constexpr i32 NoiseSizeY = 256;

constexpr f32 TerrainHeightScale = 12.0f;
constexpr f32 TerrainNoisePersistence = .46f;
constexpr i32 TerrainNoiseOctaves = 10;
constexpr i32 TerrainSeed = 6;

static bool didGenTerrain;
static f32 terrainYPositon = 0.0f;

static std::shared_ptr<InstancedBuffer> treesInstanceBuffer = std::make_shared<InstancedBuffer>();


void LoadMaterials(AssetManager& assets) {
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
}


void LoadMeshes(AssetManager& assets) {
    std::vector<ObjectDefinition> definitions = {};
    // Table
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
    // Tree 3
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
        vec3 pos = {-0.0f, 31.0f, 0.0f};
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
    // Lamp 3
    {
        vec3 pos = {3.0, 3.0f, posLampsZ};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        const float sf = 0.01f;
        vec3 scale = {sf, sf, sf};
        AddNewCustomObject_SingleMesh("Lamp 3", assets, scene, hMeshLamp, h_mat_Metal, pos, rot, scale);
    }
}


void UpdateTerrainYPos(GameScene& scene) {
    Terrain& terrain = scene.terrain;
    Transform& transform = scene.transforms.GetItemRef(terrain.hTransform);
    transform.position[1] = terrainYPositon;
}



void RebuildTerrain(AssetManager& assets, GameScene& scene) {

    UpdateTerrainYPos(scene);
    Terrain& terrain = scene.terrain;


    std::cout << "Octaves: " << terrain.heightData.octaves << std::endl;
    std::cout << "Size: (" << terrain.heightData.sizeX << ", " << terrain.heightData.sizeY << ")" << std::endl;
    std::cout << "HeightScale: " << terrain.heightData.scale << std::endl;
    std::cout << "seed: " << terrain.heightData.seed << std::endl;

    NoiseGenerator::GeneratePerlin(terrain.heightData);
    NoiseGenerator::GenerateSplatMapForTerrain(terrain.terrainSplat, terrain.heightData);

    Texture& noiseTexture = assets.textures.GetItemRef(terrain.hNoiseTex);
    Texture& splatTexture = assets.textures.GetItemRef(terrain.hSplatMapTex);
    noiseTexture.SetHeightMapData(terrain.heightData);
    splatTexture.SetSplatMapData(terrain.terrainSplat);
    noiseTexture.UploadNewTextureToGL(false);
    splatTexture.UploadNewTextureToGL(false);

}



void BuildTerrain(AssetManager& assets, GameScene& scene) {

    Terrain& terrain = scene.terrain;
    terrain.worldSize = 1024;
    terrain.heightData.sizeX = NoiseSizeX;
    terrain.heightData.sizeY = NoiseSizeY;
    terrain.heightData.scale = TerrainHeightScale;
    terrain.heightData.octaves = TerrainNoiseOctaves;
    terrain.heightData.persistence = TerrainNoisePersistence;
    terrain.heightData.seed = TerrainSeed;

    terrain.terrainSplat.band1 = 0.1f;
    terrain.terrainSplat.band2 = 0.3f;
    terrain.terrainSplat.band3 = 0.5f;
    terrain.terrainSplat.band4 = 0.7f;

    NoiseGenerator::GeneratePerlin(terrain.heightData);
    NoiseGenerator::GenerateSplatMapForTerrain(terrain.terrainSplat, terrain.heightData);

    Texture& noiseTexture = assets.textures.GetNewObjectAndHandle(terrain.hNoiseTex);
    Texture& splatTexture = assets.textures.GetNewObjectAndHandle(terrain.hSplatMapTex);
    noiseTexture.name = noiseTexName;
    splatTexture.name = splatMapTexName;
    noiseTexture.SetHeightMapData(terrain.heightData);
    splatTexture.SetSplatMapData(terrain.terrainSplat);
    noiseTexture.UploadNewTextureToGL(false);
    splatTexture.UploadNewTextureToGL(false);

    // Terrain textures
    {
        Texture& texture = assets.textures.GetNewObjectAndHandle(h_sandTex);
        assets.LoadTextureAtPath(texture, "Terrain/SandLarge_a.png", true);
        texture.name = "terrain_sand";
    }
    {
        Texture& texture = assets.textures.GetNewObjectAndHandle(h_grassTex);
        assets.LoadTextureAtPath(texture, "Terrain/Terrain_Grass_A.png", true);
        texture.name = "terrain_grass";
    }
    {
        Texture& texture = assets.textures.GetNewObjectAndHandle(h_rockTex);
        assets.LoadTextureAtPath(texture, "Terrain/gravel_1k.jpg", true);
        texture.name = "terrain_rock";
    }

    // Terrain Material
    {
        auto& material = assets.materials.GetNewObjectAndHandle(h_mat_Terrain);
        material.shaderName = "Terrain";

        material.SetTextureDefinition(TEX_HEIGHT_MAP, noiseTexName, 0);
        material.SetTextureDefinition(TEX_SPLAT_MAP, splatMapTexName, 1);

        material.SetTextureDefinition(TEX_TERRAIN_TEX_1, "terrain_rock", 2);
        material.SetTextureDefinition(TEX_TERRAIN_TEX_2, "terrain_sand", 3);
        material.SetTextureDefinition(TEX_TERRAIN_TEX_3, "terrain_grass", 4);
        material.SetTextureDefinition(TEX_TERRAIN_TEX_4, "terrain_rock", 5);

        material.SetFloatDefinition(TERRAIN_HEIGHT_SCALE, TerrainHeightScale);
        material.SetFloatDefinition(ID_SMOOTHNESS, 0.01f);
        constexpr float shade = .5f;
        material.SetVectorDefinition(ID_COLOR_TINT, {shade, shade, shade, 1.0});

        GL_InitMaterialProperties(material, assets);
    }

    terrain.hMaterial = h_mat_Terrain;

    Transform& transform = scene.transforms.GetNewObjectAndHandle(terrain.hTransform);
    Transform_Init(transform);
    SET_VEC3(transform.position, 0.0f, 0.0f, 0.0f);
    Transform_SetRotationEulerDeg(transform, 0.0, 0.0, 0.0);
    Transform_UpdateMatrices(transform);

    terrain.GenerateMeshData();
    GL_AllocateGraphicsTerrain(terrain);
}



void ScriptTerrainGame::RegenTerrain() {
    auto& engine = *Engine::GetInstance();
    if (!didGenTerrain)
        std::cout << "Generating Terrain First Time" << std::endl;
    else
        std::cout << "Rebuilding Terrain" << std::endl;

    if (!didGenTerrain) {
        didGenTerrain = true;
        BuildTerrain(engine.assetManager, engine.scene);
    }
    else {
        RebuildTerrain(engine.assetManager, engine.scene);
        // PlaceTreesOnTerrain(engine.scene.terrain, engine.scene.transforms.GetItemRef(engine.scene.terrain.hTransform));
    }
}


void PlaceObjectsToScene(AssetManager& assets, GameScene& scene) {
    // //Floor
    // {
    //     vec3 pos = {0.0, -0.5f, 0.0f};
    //     vec3 rot = {0.0f, 0.0f, 0.0f};
    //     vec3 scale = {32.0f, 1.0f, 32.0f};
    //     Handle objHandle = scene.NewObject_SingleSubMesh("FloorPlane", assets.meshCube, pos, rot, scale, h_mat_Floor);
    //     scene.activeWorldHandles.push_back(objHandle);
    // }
    // PlaceDefaultShapes(assets, scene);
    PlaceCustomShapes(assets, scene);
}



void PlaceCamera(GameScene& scene) {
    Transform& cameraTransform = scene.transforms.GetNewObjectAndHandle(scene.camera.transformHandle);

    Transform_SetWorldPosition(cameraTransform, 0.0f, 2.0f, -5.0f);
    Transform_SetRotationEulerDeg(cameraTransform, 10.0f, 0.0f, 0.0f);
    Transform_SetLocalScale(cameraTransform, 1.0f, 1.0f, 1.0f);
}



void InitSceneLights(GameScene& scene){
    Light& light = scene.mainLight;
    Transform& lightTransform = scene.transforms.GetNewObjectAndHandle(light.transformHandle);
    light.lightType = ELightType::Directional;

    vec3 pos = {0.0f, 10.0f, -20.0f};
    vec3 eulers = {0.0f, 0.0f, 0.0f};
    vec3 scale = {LightDebugScale, LightDebugScale, LightDebugScale};
    Transform_Init(lightTransform);
    Transform_SetLocalPositionRotationScale(lightTransform, pos, eulers, scale);
    Transform_RotateWorldY(lightTransform, 30.0);
    Transform_RotateLocalX(lightTransform, 50.0);
}




void ScriptTerrainGame::PlaceTreesOnTerrain(Terrain& terrain, Transform& terrainTransform) {
    if (treesInstanceBuffer == nullptr) {
        std::cerr << "treesInstanceBuffer is null" << std::endl;
        return;
    }
    auto& buffer = *treesInstanceBuffer;
    auto size = buffer.entries.size();

    vec3 terrainOriginPosition;
    terrain.GetVertexOriginPosition(terrainTransform.position, terrainOriginPosition);
    f32 yOffset = terrainTransform.position[1];
    for (auto i = 0; i < size; i++) {
        vec3 position;
        glm_vec3_copy(buffer.entries[i].modelMatrix[3], position);
        glm_vec3_sub(position, terrainOriginPosition, position);
        buffer.entries[i].modelMatrix[3][1] = terrain.GetHeightAt(position[0], position[2]) + yOffset;
    }
    std::cout << std::endl<< std::endl<< std::endl;
    buffer.isDirty = true;
}



void ScriptTerrainGame::InitTreesInstanceBuffer() {
    auto& engine = *Engine::GetInstance();
    auto& terrain = engine.scene.terrain;
    auto& terrainTransform = engine.scene.transforms.GetItemRef(engine.scene.terrain.hTransform);

    const i32 count = 900;
    const i32 entriesPerRow = 90;
    const f32 spacing = 12.0f;
    f32 startX = -1.0f * entriesPerRow * .5f * spacing;
    f32 startY = -1.0f * entriesPerRow * .5f * spacing;

    auto& buffer = *treesInstanceBuffer;
    buffer.name = "Trees";
    buffer.renderData.hMesh = hMeshTree;
    buffer.renderData.hMaterial = h_mat_Tree;
    GL_AllocateGraphicsMesh(buffer.renderData, engine.assetManager);
    buffer.entries.resize(count);

    vec3 terrainOriginPosition;
    terrain.GetVertexOriginPosition(terrainTransform.position, terrainOriginPosition);

    f32 yOffset = terrainTransform.position[1];
    for (size_t i = 0; i < count; i++) {
        i32 x = i % entriesPerRow;
        i32 z = (i / entriesPerRow);

        vec3 position = {
            startX + x * spacing,
            0,
            startY + z * spacing};

        vec3 relativePos;
        glm_vec3_sub(position, terrainOriginPosition, relativePos);
        position[1] = terrain.GetHeightAt(relativePos[0], relativePos[2]) + yOffset;

        versor rotation = {0.0f, 0.0f, 0.0f, 1.0f};
        vec3 scale = {1., 1., 1.};

        glm_vec3_scale(scale, 0.01f, scale);

        Transform_UpdateMatrixOnly(buffer.entries[i].modelMatrix, position, rotation, scale);
    }
    GL_AddInstanceBuffer(treesInstanceBuffer);
}




void InitNoiseDebugUI(Engine& engine) {
    UIObject& obj = engine.scene.uiObjectsPool.GetNewObjectAndHandle(h_objUIQuad);
    Material& uiMat = engine.assetManager.materials.GetNewObjectAndHandle(h_matUI);
    uiMat.shaderName = "UIQuad";
    uiMat.vectorsDefinitions.clear();
    uiMat.floatsDefinitions.clear();
    uiMat.texturesDefinitions.clear();
    obj.hMaterial = h_matUI;

    SET_VEC2(obj.position, 10, 10);
    SET_VEC2(obj.size, 200, 200);
    GL_AllocateGUIQuad(obj);
    engine.scene.activeUIHandles.push_back(h_objUIQuad);

    uiMat.SetTextureDefinition(ID_MAIN_TEXTURE, splatMapTexName, 0);
    GL_InitMaterialProperties(uiMat, engine.assetManager);
}



void ScriptTerrainGame::Create() {
}

void ScriptTerrainGame::Start() {
    auto& engine = *Engine::GetInstance();

    LoadMeshes(engine.assetManager);
    LoadMaterials(engine.assetManager);
    PlaceCamera(engine.scene);
    InitSceneLights(engine.scene);
    PlaceObjectsToScene(engine.assetManager, engine.scene);
    RegenTerrain();

    InitTreesInstanceBuffer();

    InitNoiseDebugUI(engine);
}


void ScriptTerrainGame::GamePause() {
}
void ScriptTerrainGame::GameResume() {
}
void ScriptTerrainGame::Quit() {
}
void ScriptTerrainGame::PhysicsUpdate(f32 deltaTime) {
}
void ScriptTerrainGame::Update(f32 deltaTime) {
}


void ScriptTerrainGame::GUIUpdate(f32 deltaTime) {

    auto& engine = *Engine::GetInstance();
    auto& terrain = engine.scene.terrain;

    // Sliders automatically update the variables passed by reference
    ImGui::Begin("Terrain Controls");
    ImGui::SliderInt("Noise Octaves", &terrain.heightData.octaves, 1, 15);
    ImGui::SliderFloat("Height Scale", &terrain.heightData.scale, 1.0f, 20.0f);
    ImGui::SliderInt("Seed", &terrain.heightData.seed, 0, 10);
    ImGui::SliderFloat("Persistence", &terrain.heightData.persistence, 0.0f, 1.0f);

    ImGui::SliderFloat("Y Offset", &terrainYPositon, -250.0f, 250.0f);

    ImGui::BulletText("Splat Map Settings");
    ImGui::SliderFloat("Band 1 (rock1)", &terrain.terrainSplat.band1,  0.0, 1.0);
    ImGui::SliderFloat("Band 2 (sand)", &terrain.terrainSplat.band2,  0.0, 1.0);
    ImGui::SliderFloat("Band 3 (grass)", &terrain.terrainSplat.band3,  0.0, 1.0);
    ImGui::SliderFloat("Band 4 (rock2)", &terrain.terrainSplat.band4,  0.0, 1.0);
    // Adds visual spacing before the button
    ImGui::Spacing();
    ImGui::Separator();
    ImGui::Spacing();

    ImGui::SliderInt("LODs count",  &terrain.LODS, 0, 10.0);
    ImGui::SliderInt("Offset X",  &terrain.textOffsetX, -12.0, 12.0);
    ImGui::SliderInt("Offset Z",  &terrain.textOffsetZ, -12.0, 12.0);
    ImGui::Checkbox( "Debug snapping",  &terrain.debugSnapping);

    // Button returns true exactly on the frame it is clicked
    if (ImGui::Button("Generate Terrain", ImVec2(150, 30))) {
        RegenTerrain();
    }

    ImGui::End();

    UpdateTerrainYPos(engine.scene);
}

