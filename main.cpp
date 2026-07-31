#include "SceneDefinition.h"
#include <iostream>
#include <vector>
#include <filesystem>
#include "InputSystem.h"
#include "Shader.h"
#include "Texture.h"
#include "GameTime.h"
#include "Transform.h"
#include "GameScene.h"
#include "Camera.h"
#include "Light.h"
#include "cglm/cglm.h"
#include "cglm/clipspace/persp_lh_no.h"
#include "RenderObject.h"
#include "AssetManager.h"
#include "SlotsMap.h"
#include "GraphicsGL.h"
#include "RenderSubMesh.h"
#include "Uniforms.h"
#include "Skybox.h"
#include "Engine.h"
#include "ShaderWatcher.h"
#include "ProgramWindow.h"
#include "myGui.h"
#include <windows.h>

#define LOG(str) do {printf("%s\n", str);} while(false);
#define LOG2(str1, str2) do {printf("%s1 %s2\n", str1, str2); } while(false);
#define LOG3(str1, str2, str3) do {printf("%s1 %s2 %s3\n", str1, str2, str3); } while(false);

#define STR_VEC2(vec) "[" << vec[0]<< ", " << vec[1] << "]";
#define STR_VEC3(vec) "[" << vec[0]<< ", " << vec[1] << ", " << vec[2] << "]";
#define STR_VEC4(vec) "[" << vec[0]<< ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << "]";

// region Static Data
static ProgramWindow mainWin = {};

static Engine* EnginePtr = nullptr;

const char* SkyBoxName = "Skybox";


void AddDebugGeometryForLights(GameScene& scene, AssetManager& assets) {
    {
        vec3 pos, eulers, scale;
        Handle objHandle = scene.NewObject_SingleSubMesh("Light Debug", assets.meshCutCone, pos, eulers, scale, assets.materialDebug);
        scene.activeWorldHandles.push_back(objHandle);
        auto& newObj = scene.worldObjectsPool.GetItemRef(objHandle);
        scene.transforms.FreeHandle(newObj.hTransform);
        newObj.hTransform = scene.mainLight.transformHandle;
    }
}


void LoadSkybox(Skybox& skyBox) {
    GL_AllocateGraphicsSkybox(skyBox.renderData);
    auto& assets = EnginePtr->assetManager;
    Material& material = assets.materials.GetItemRef(assets.materialSkybox);
    skyBox.renderData.hMaterial = assets.materialSkybox;
    GL_InitMaterialProperties(material, assets);
}


void LoadShadersDeferred(Engine& engine) {
    auto& assets = engine.assetManager;

    Shader& default3DG = assets.shaders.GetNewObjectAndHandle(assets.shaderDefault3D);
    Shader& default3DL = assets.shaders.GetNewObjectAndHandle(assets.shaderDefault3DLight);
    Shader& skybox = assets.shaders.GetNewObjectAndHandle(assets.materialSkybox);
    // post process shader
    Shader& ssrShader = assets.shaders.GetNewObjectAndHandle(assets.shaderSSR);
    Shader& bloomDown1 = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomDownsampleFirst);
    Shader& bloomDown2 = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomDownsample);
    Shader& bloomUp = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomUpSample);
    Shader& bloomComposite = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomComposite);
    Shader& renderTexture = assets.shaders.GetNewObjectAndHandle(assets.shaderScreenRenderTexture);
    Shader& terrain = assets.shaders.GetNewObjectAndHandle(assets.shaderTerrain);
    Shader& uiQuad = assets.shaders.GetNewObjectAndHandle(assets.uiQuad);

    // Assign names
    default3DG.SetNameSeparate(Shader_DefaultDeferredG, "Deferred/Deferred_G_3D", "Deferred/Deferred_G_3D");
    default3DL.SetNameSeparate(Shader_DefaultDeferredL, "ScreenRenderTexture", "Deferred/Deferred_L_3D");
    skybox.SetNameSeparate(Shader_SkyboxDefault, "Deferred/Skybox", "Deferred/Skybox");

    renderTexture.SetName(Shader_ScreenRender);
    ssrShader.SetNameSeparate("SSR", "ScreenRenderTexture", "PostProcess/SSR");
    bloomDown1.SetNameSeparate("BloomDownsampleFirstPass", "ScreenRenderTexture", "PostProcess/BloomDownsampleFirstPass");
    bloomDown2.SetNameSeparate("BloomDownsample", "ScreenRenderTexture", "PostProcess/BloomDownsample");
    bloomUp.SetNameSeparate("BloomUpsample", "ScreenRenderTexture", "PostProcess/BloomUpsample");
    bloomComposite.SetNameSeparate("BloomComposition", "ScreenRenderTexture", "PostProcess/BloomComposite");
    terrain.SetNameSeparate("Terrain", "Deferred/Terrain_G", "Deferred/Terrain_G");
    uiQuad.SetName("UIQuad");

    bool allCompiled = true;
    allCompiled |= default3DG.LoadAndCompile() == 0;
    allCompiled |= default3DL.LoadAndCompile() == 0;
    allCompiled |= skybox.LoadAndCompile() == 0;

    allCompiled |= renderTexture.LoadAndCompile() == 0;
    allCompiled |= ssrShader.LoadAndCompile() == 0;
    allCompiled |= bloomDown1.LoadAndCompile() == 0;
    allCompiled |= bloomDown2.LoadAndCompile() == 0;
    allCompiled |= bloomUp.LoadAndCompile() == 0;
    allCompiled |= bloomComposite.LoadAndCompile() == 0;
    allCompiled |= terrain.LoadAndCompile() == 0;
    allCompiled |= uiQuad.LoadAndCompile() == 0;


#define LOG_DEFAULT_SHADER_COMP
#ifdef LOG_DEFAULT_SHADER_COMP
    std::cout << "[shader] compiled default3DG " << default3DG.GetName() << " " << default3DG.GetShaderId() << std::endl;
    std::cout << "[shader] compiled default3DL " << default3DL.GetName() << " " << default3DL.GetShaderId() << std::endl;
    std::cout << "[shader] compiled skybox " << skybox.GetName() << " " << skybox.GetShaderId() << std::endl;
    std::cout << "[shader] compiled terrain " << terrain.GetName() << " " << terrain.GetShaderId() << std::endl;
    std::cout << "[shader] compiled uiQuad " << uiQuad.GetName() << " " << uiQuad.GetShaderId() << std::endl;
#endif

    assert(allCompiled);

    engine.shaderWatcher.WatchShader(default3DG);
    engine.shaderWatcher.WatchShader(default3DL);
    engine.shaderWatcher.WatchShader(skybox);

    engine.shaderWatcher.WatchShader(renderTexture);
    engine.shaderWatcher.WatchShader(ssrShader);
    engine.shaderWatcher.WatchShader(bloomDown1);
    engine.shaderWatcher.WatchShader(bloomDown2);
    engine.shaderWatcher.WatchShader(bloomUp);
    engine.shaderWatcher.WatchShader(bloomComposite);
    engine.shaderWatcher.WatchShader(terrain);
    engine.shaderWatcher.WatchShader(uiQuad);

    // Additional Shaders
    {
        Handle h {};
        Shader& treeGeom = assets.shaders.GetNewObjectAndHandle(h);
        treeGeom.SetNameSeparate("Tree", "Deferred/Tree", "Deferred/TreeG");
        treeGeom.LoadAndCompile();
        engine.shaderWatcher.WatchShader(treeGeom);

    }
}


void LoadShaders(Engine& engine) {

    auto& assets = engine.assetManager;
    Shader& default3D = assets.shaders.GetNewObjectAndHandle(assets.shaderDefault3D);
    Shader& default2D = assets.shaders.GetNewObjectAndHandle(assets.shaderDefault2D);
    Shader& debugShader = assets.shaders.GetNewObjectAndHandle(assets.shaderLightDebug);
    Shader& defaultSkybox = assets.shaders.GetNewObjectAndHandle(assets.shaderDefaultSkybox);

    // Dev staff
    Shader& depthOnly = assets.shaders.GetNewObjectAndHandle(assets.shaderDepthOnly);
    Shader& normalsOnly = assets.shaders.GetNewObjectAndHandle(assets.shaderNormalsOnly);
    Shader& colorOnly = assets.shaders.GetNewObjectAndHandle(assets.shaderColorOnly);

    // post process shader
    Shader& renderTexture = assets.shaders.GetNewObjectAndHandle(assets.shaderScreenRenderTexture);
    Shader& ssrShader = assets.shaders.GetNewObjectAndHandle(assets.shaderSSR);
    Shader& bloomDown1 = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomDownsampleFirst);
    Shader& bloomDown2 = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomDownsample);
    Shader& bloomUp = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomUpSample);
    Shader& bloomComposite = assets.shaders.GetNewObjectAndHandle(assets.shaderBloomComposite);

    default3D.SetName(Shader_DefaultForward);
    default2D.SetName(Shader_Default2D);
    debugShader.SetName(Shader_Debug);
    defaultSkybox.SetName(Shader_SkyboxDefault);

    renderTexture.SetName(Shader_ScreenRender);
    ssrShader.SetNameSeparate("SSR", "ScreenRenderTexture", "PostProcess/SSR");
    bloomDown1.SetNameSeparate("BloomDownsampleFirstPass", "ScreenRenderTexture", "PostProcess/BloomDownsampleFirstPass");
    bloomDown2.SetNameSeparate("BloomDownsample", "ScreenRenderTexture", "PostProcess/BloomDownsample");
    bloomUp.SetNameSeparate("BloomUpsample", "ScreenRenderTexture", "PostProcess/BloomUpsample");
    bloomComposite.SetNameSeparate("BloomComposition", "ScreenRenderTexture", "PostProcess/BloomComposite");

    depthOnly.SetNameSeparate("DepthOnly", "ScreenRenderTexture", "Dev/DepthOnly");
    normalsOnly.SetNameSeparate("NormalsOnly", "ScreenRenderTexture", "Dev/DepthOnly");
    colorOnly.SetNameSeparate("ColorsOnly", "ScreenRenderTexture", "Dev/DepthOnly");

    bool allCompiled = true;
    // Actually compile them
    allCompiled |= default3D.LoadAndCompile() == 0;

    allCompiled |= default2D.LoadAndCompile() == 0;
    allCompiled |= debugShader.LoadAndCompile() == 0;
    allCompiled |= defaultSkybox.LoadAndCompile() == 0;

    allCompiled |= renderTexture.LoadAndCompile() == 0;
    allCompiled |= ssrShader.LoadAndCompile() == 0;
    allCompiled |= bloomDown1.LoadAndCompile() == 0;
    allCompiled |= bloomDown2.LoadAndCompile() == 0;
    allCompiled |= bloomUp.LoadAndCompile() == 0;
    allCompiled |= bloomComposite.LoadAndCompile() == 0;

    allCompiled |= depthOnly.LoadAndCompile() == 0;
    allCompiled |= normalsOnly.LoadAndCompile() == 0;
    allCompiled |= colorOnly.LoadAndCompile() == 0;

#ifdef LOG_DEFAULT_SHADER_COMP

    std::cout << "[shader] compiled " << default3D.GetName() << " " << default3D.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << default2D.GetName() << " " << default2D.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << debugShader.GetName() << " " << debugShader.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << defaultSkybox.GetName() << " " << defaultSkybox.GetShaderId() << std::endl;

    std::cout << "[shader] compiled " << ssrShader.GetName() << " " << ssrShader.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << bloomDown1.GetName() << " " << bloomDown1.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << bloomDown2.GetName() << " " << bloomDown2.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << bloomUp.GetName() << " " << bloomUp.GetShaderId() << std::endl;

    //std::cout << "[shader] compiled " << bloomShader.GetName() << " " << bloomShader.GetShaderId() << std::endl;
    //std::cout << "[shader] compiled " << blurShader.GetName() << " " << blurShader.GetShaderId() << std::endl;
    //std::cout << "[shader] compiled " << colorShader.GetName() << " " << colorShader.GetShaderId() << std::endl;
#endif

    assert(allCompiled);

    engine.shaderWatcher.WatchShader(default3D);
    engine.shaderWatcher.WatchShader(default2D);
    engine.shaderWatcher.WatchShader(debugShader);
    engine.shaderWatcher.WatchShader(defaultSkybox);

    engine.shaderWatcher.WatchShader(renderTexture);
    engine.shaderWatcher.WatchShader(ssrShader);
    engine.shaderWatcher.WatchShader(bloomDown1);
    engine.shaderWatcher.WatchShader(bloomDown2);
    engine.shaderWatcher.WatchShader(bloomUp);
    engine.shaderWatcher.WatchShader(bloomComposite);

    // Additional shaders
    {
        Handle h {};
        Shader& treeShader = assets.shaders.GetNewObjectAndHandle(h);
        treeShader.SetName("Tree");
        treeShader.LoadAndCompile();
        treeShader.SetAcceptsLighting(true);
        engine.shaderWatcher.WatchShader(treeShader);
    }
}


void LoadSkyboxTexture(const char* textureName) {
    Handle h = {};
    Texture& texture = EnginePtr->assetManager.GetNewTextureObject(h);
    texture.name = textureName;
    texture.pixelFormat = 1;
    std::vector<std::string> facePaths = {};
    facePaths.reserve(6);
    for (size_t i = 0; i < 6; ++i) {
        facePaths.emplace_back();
    }
    facePaths[0] = std::string(textureName) + "/nx.png";
    facePaths[1] = std::string(textureName) + "/px.png";
    facePaths[2] = std::string(textureName) + "/py.png";
    facePaths[3] = std::string(textureName) + "/ny.png";
    facePaths[4] = std::string(textureName) + "/nz.png";
    facePaths[5] = std::string(textureName) + "/pz.png";

    AssetManager::LoadTextureCubeMap6Face(texture, facePaths);
}


void LoadDefaultTextures() {
    AssetManager& assets = EnginePtr->assetManager;
    assets.CreateDefaultWhiteTexture();
    assets.CreateDefaultNormalMap();
    LoadSkyboxTexture(SkyBoxName);
}

void LoadDefaultMeshes() {
    auto& assets = EnginePtr->assetManager;
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshCube);
        Mesh_DefaultCube(mesh);
        mesh.name = MESH_CUBE;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshSphere);
        Mesh_DefaultSphere(mesh);
        mesh.name = MESH_SPHERE;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshCapsule);
        Mesh_DefaultCapsule(mesh);
        mesh.name = MESH_CAPSULE;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshDonut);
        Mesh_DefaultDonut(mesh);
        mesh.name = MESH_DONUT;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshPlane);
        Mesh_DefaultPlane(mesh);
        mesh.name = MESH_PLANE;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshQuad);
        Mesh_DefaultQuad(mesh);
        mesh.name = MESH_QUAD;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshPyramid);
        Mesh_DefaultPyramid(mesh);
        mesh.name = MESH_PYRAMID;
    }
    {
        Mesh& mesh = assets.meshes.GetNewObjectAndHandle(assets.meshCutCone);
        Mesh_DefaultCutCone(mesh);
        mesh.name = MESH_CUT_CONE;
    }
}

void InitCamera() {
    Camera& camera = EnginePtr->scene.camera;
    Transform& cameraTransform = EnginePtr->scene.transforms.GetNewObjectAndHandle(camera.transformHandle);
    Transform_Init(cameraTransform);
}



// region Loops

void StartFrame() {
    Camera& camera = EnginePtr->scene.camera;
    camera.UpdateAspectRationWidthHeight(static_cast<float>(mainWin.width), static_cast<float>(mainWin.height));
    Transform& cameraTransform = EnginePtr->scene.transforms.GetItemRef(camera.transformHandle);
    camera.UpdateMatrices(cameraTransform);
    EnginePtr->gui.StartFrame();

}

void EndFrame() {
    SwapBuffers(mainWin.dc);
    EnginePtr->shaderWatcher.ProcessReloads();
    EnginePtr->gui.EndFrame();
}


void UpdateSceneTransforms(GameScene& scene) {
    std::vector<Transform>& allTransforms = scene.transforms.GetVector();
    for (Transform& temp : allTransforms) {
        Transform_UpdateMatrices(temp);
    }
}



void RenderLoop() {
    Engine& eng = *EnginePtr;

    UpdateSceneTransforms(eng.scene);
    StartFrame();
    GL_RenderScene(eng);

    EndFrame();
}
// endregion

static float elapsedTimeMoving = 0.0f;

void ControlCamera() {
    float dt = (float)Time_GetDelta();
    Transform& cameraTransform = EnginePtr->scene.transforms.GetItemRef(EnginePtr->scene.camera.transformHandle);
    float moveSpeed = EnginePtr->settings.Camera_Move_Speed;
    float rotSpeed  = EnginePtr->settings.Camera_Rotation_Speed;

    vec3 localMove = {};
    float verticalShift = 0;
    if (Input_IsKeyHeld(GameInputKey::KEY_W)) {
        localMove[2] = 1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_A)) {
        localMove[0] = -1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_S)) {
        localMove[2] = -1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_D)) {
        localMove[0] = 1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_E)) {
        verticalShift = 1;
    }
    if (Input_IsKeyHeld(GameInputKey::KEY_Q)) {
        verticalShift = -1;
    }
    vec2 mousePosition;
    Input_GetMousePosition(mousePosition);
    bool isMoving = localMove[0] != 0 || localMove[1] != 0 || localMove[2] != 0;
    if (isMoving) {
        elapsedTimeMoving += Time_GetDelta();
    }
    else{
        elapsedTimeMoving = 0.0;
    }

    if (Input_IsMouseButtonHeld(GameInputKey::MOUSE_BUTTON_RIGHT)) {
        vec2 mouseDelta;
        Input_GetMouseDelta(mouseDelta);
        glm_vec2_scale(mouseDelta, dt * rotSpeed, mouseDelta);

        vec3 eulersBefore;
        vec3 eulersAfter;
        Transform_QuatToEuler(cameraTransform.rotation, eulersBefore);

        Transform_RotateWorldY(cameraTransform, mouseDelta[0]);
        Transform_RotateLocalX(cameraTransform, -mouseDelta[1]);
        Transform_QuatToEuler(cameraTransform.rotation, eulersAfter);
    }

    vec3 worldMove;
    Transform_ToWorldVector(cameraTransform, localMove, worldMove);
    glm_vec3_scale(worldMove, dt * moveSpeed * (1 + elapsedTimeMoving), worldMove);

    vec3 verticalMove = {0,1,0};
    glm_vec3_scale(verticalMove, dt * moveSpeed * verticalShift, verticalMove);
    glm_vec3_add(worldMove, verticalMove, worldMove);
    glm_vec3_add(cameraTransform.position, worldMove, cameraTransform.position);
}

void ControlSettings() {
    ProjectSettings& settings = EnginePtr->settings;

    if (Input_IsKeyDown(GameInputKey::KEY_1)) {
        settings.UseGammaCorrection  = !settings.UseGammaCorrection;
        std::cout << "[GammaCorrection]: " << settings.UseGammaCorrection << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_P)) {
        settings.DevRenderDepths  = !settings.DevRenderDepths;
        std::cout << "[Dev Depths Only]: " << settings.DevRenderDepths << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_N)) {
        settings.DevRenderNormals  = !settings.DevRenderNormals;
        std::cout << "[Dev Normals Only]: " << settings.DevRenderNormals << std::endl;
    }

    if (Input_IsKeyDown(GameInputKey::KEY_J)) {
        settings.RenderSkyBox  = !settings.RenderSkyBox;
        std::cout << "[Render Skybox]: " << settings.RenderSkyBox << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_K)) {
        settings.RenderShadows  = !settings.RenderShadows;
        std::cout << "[Render Shadows]: " << settings.RenderShadows << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_L)) {
        settings.PostProcess  = !settings.PostProcess;
        std::cout << "[Render PostProcess]: " << settings.PostProcess << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_B)) {
        settings.PostProcess_Bloom  = !settings.PostProcess_Bloom;
        std::cout << "[Render PostProcess_Bloom]: " << settings.PostProcess_Bloom << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_R)) {
        settings.PostProcess_SSR  = !settings.PostProcess_SSR;
        std::cout << "[Render PostProcess_SSR]: " << settings.PostProcess_SSR << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_O)) {
        settings.PostProcess_DOF  = !settings.PostProcess_DOF;
        std::cout << "[Render PostProcess_DOF]: " << settings.PostProcess_DOF << std::endl;
    }
    if (Input_IsKeyDown(GameInputKey::KEY_P)) {
        settings.PostProcess_ToneMapping  = !settings.PostProcess_ToneMapping;
        std::cout << "[Render PostProcess_DOF]: " << settings.PostProcess_ToneMapping << std::endl;
    }
}


void ControlsLoop() {

    ControlCamera();
    ControlSettings();
}


// region WinProc callbacks and Inputs
int Win_CloseWindow() {
    mainWin.close = true;
    PostQuitMessage(0);
    return 0;
}


int Win_Move(uint32_t newWidth, uint32_t newHeight) {
    mainWin.posX = newWidth;
    mainWin.posY = newHeight;
    return 0;
}

int Win_Resize(uint32_t newWidth, uint32_t newHeight) {
    bool didChange = false;
    if (mainWin.width != newWidth) {
        mainWin.width = newWidth;
        didChange = true;
    }
    if (mainWin.height != newHeight) {
        mainWin.height = newHeight;
        didChange = true;
    }
    if (didChange) {
        GL_ResizeRenderTarget(newWidth, newHeight);
    }
    return 0;
}
// endregion


void FetchProjectPath(std::string &exePath, std::string &resourcesPat) {
    char buffer[1024];
    GetModuleFileNameA(NULL, buffer, sizeof(buffer)); // WIN 32 API for current .exe
    // std::string fullPath = buffer;
    std::filesystem::path fullPath = buffer;
    std::filesystem::path _exePath = fullPath.parent_path();
    std::filesystem::path _resourcesPat = _exePath.parent_path();
    exePath = _exePath.string();
    resourcesPat = (_resourcesPat / "Resources").string();
}


void InitDefaults() {
    Engine& engine = *EnginePtr;

    Time_Init();
    Time_SetTargetFrameRate(60);
    GL_InitGraphics(mainWin.width, mainWin.height);

    LoadShadersDeferred(engine);
    LoadDefaultMeshes();
    LoadDefaultTextures();

    GL_InitDefaultMaterials(engine.assetManager);
    LoadSkybox(engine.scene.skybox);
    InitCamera();

    RunGameSceneInit();
    AddDebugGeometryForLights(engine.scene, engine.assetManager);
}



void MakeConsole() {
    if (AllocConsole()) {
        FILE* fpOut;
        freopen_s(&fpOut, "CONOUT$", "w", stdout);
        FILE* fpErr;
        freopen_s(&fpErr, "CONOUT$", "w", stderr);
        std::ios::sync_with_stdio(true);
        SetConsoleTitle(TEXT("Console Output Window"));
    }
    else {
        printf("Failed to allocate console");
    }
}



void CreateEngine() {
    EnginePtr = Engine::GetInstance();
    auto& engine = *EnginePtr;
    InitDefaults();
    engine.UpdateSettings();
    engine.shaderWatcher.WatchSettingsFile();
    engine.gui.InitForWindow(&mainWin);
}



bool CreateMainWindow(ProgramWindow& window, HINSTANCE hInstance) {
    MakeConsole();
    FetchProjectPath(ProjectSettings::RootPath, ProjectSettings::ResourcesPath);

    window.name = "Renderer Window";
    window.width = 1024;
    window.height = 700;
    window.posX = 612;
    window.posY = 100;
    window.hInst = hInstance;
    window.callbackClose = Win_CloseWindow;
    window.callbackResize = Win_Resize;
    window.callbackMove = Win_Move;
    window.callbackResize = Win_Resize;

    bool didInit = window.CreateNativeWindowOpenGL();
    if (didInit == false) {
        std::cerr << "FAILED TO LOAD WIN AND GL\n";
        return false;
    }
    return true;
}


void RunSingleLoop() {
    // printf("Frame %d, Delta: %f \n", Time_GetFrameCountInt(), Time_GetDelta());
    Time_Update();

    mainWin.ProcessEvents();
    EnginePtr->gui.UpdateInputs();

    RenderLoop();
    ControlsLoop();

    Input_Update();
}


void MainLoop() {
    while (!mainWin.close)
    {
        try {
            RunSingleLoop();
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
}



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd) {

    bool didCreate = CreateMainWindow(mainWin, hInstance);
    if (!didCreate) {
        return -1;
    }

    CreateEngine();

    MainLoop();
    std::cout<<"Main Loop terminated. SPIN\n";
    return 0;
}

