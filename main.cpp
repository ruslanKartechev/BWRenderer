#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <filesystem>
#include "InputSystem.h"
#include "PlatformWin32.h"
#include "Shader.h"
#include "Texture.h"
#include "GameTime.h"
#include "Transform.h"
#include "GameScene.h"
#include "Camera.h"

#include "DataStructures.h"
#include "cglm/cglm.h"
#include "cglm/clipspace/persp_lh_no.h"

#include "RenderObject.h"
#include "AssetManager.h"
#include "SlotsMap.h"
#include "GraphicsGL.h"
#include "RenderSubMesh.h"
#include "Uniforms.h"
#include "Skybox.h"
#include "ShaderWatcher.h"
#include "Engine.h"
#include "SceneDefinition.h"
#include "ProjectDefines.h"

#define LOG(str) do {printf("%s\n", str);}while(false);
#define LOG2(str1, str2) do {printf("%s1 %s2\n", str1, str2); }while(false);
#define LOG3(str1, str2, str3) do {printf("%s1 %s2 %s3\n", str1, str2, str3); }while(false);

#define STR_VEC2(vec) "[" << vec[0]<< ", " << vec[1] << "]";
#define STR_VEC3(vec) "[" << vec[0]<< ", " << vec[1] << ", " << vec[2] << "]";
#define STR_VEC4(vec) "[" << vec[0]<< ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << "]";


// region Graphics Data Structures
typedef struct {
    size_t quadCountMax;
    int currentQuadsCount;
    int idxCount;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    GLuint shaderId;
} FrameBufferUI;


// endregion


// region Static Data
static WindowParams mainWin = {};

static Engine* EnginePtr = nullptr;

const char* SkyBoxName = "Skybox";


void AddDebugGeometryForLights(GameScene& scene, AssetManager& assets) {
    {
        vec3 pos, eulers, scale;
        Handle objHandle = scene.NewObject_SingleSubMesh("Light Debug", assets.meshCutCone, pos, eulers, scale, assets.materialDebug);
        scene.existingObjects.push_back(objHandle);
        auto& newObj = scene.renderObjects.GetItemRef(objHandle);
        scene.transforms.FreeHandle(newObj.hTransform);
        newObj.hTransform = scene.mainLight.transformHandle;
    }
}



void LogHandle(const char* msg, const Handle& handle) {

    printf(msg);
    printf(" Handle(%d, %d)\n", handle.index, handle.generation);
}


void InitBackground(FrameBufferUI& fbBackground) {
    // actual background quad with UV and Color
    float vertexData[32] = {
        // x,  y,       u,    v,     r,      g,      b,      a
        -1.0f, -1.0f,   0.0f, 0.0f,  1.0f, 0.0f, 0.0f, 1.0f,
        -1.0f, +1.0f,   0.0f, 1.0f,  0.0f, 1.0f, 0.0f, 1.0f,
        +1.0f, +1.0f,   1.0f, 1.0f,  0.0f, 0.0f, 1.0f, 1.0f,
        +1.0f, -1.0f,   0.0f, 1.0f,  0.0f, 0.0f, 0.0f, 1.0f,
    };
    constexpr int startIdxCount = 6;
    unsigned int indices[startIdxCount] = {
        0, 2, 1, // Trig 1
        0, 3, 2  // Trig 2
    };

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);

    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertexData), vertexData, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);
    constexpr size_t stride = 8 * sizeof(float);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (const void*)0); // XY
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_TRUE, stride, (const void*)(2*sizeof(float))); // UV
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(4*sizeof(float))); // RGBA

    glBindVertexArray(0);

    fbBackground.vao = vao;
    fbBackground.vbo = vbo;
    fbBackground.ebo = ebo;
    fbBackground.currentQuadsCount = 2;
    fbBackground.idxCount = startIdxCount;
    fbBackground.shaderId = EnginePtr->assetManager.GetDefault2D().GetShaderId();
}



void InitSkybox(Skybox& skyBox) {
    GL_AllocateGraphicsSkybox(skyBox.renderData);
    Material& material = EnginePtr->assetManager.materials.GetItemRef(EnginePtr->assetManager.materialSkybox);
    skyBox.renderData.hMaterial = EnginePtr->assetManager.materialSkybox;
    GL_InitMaterialProperties(material, EnginePtr->assetManager);

}


void LoadDefaultShaders(Engine& engine) {

    Shader& default3D = engine.assetManager.shaders.GetNewObjectAndHandle(engine.assetManager.shaderDefault3D);
    Shader& default2D = engine.assetManager.shaders.GetNewObjectAndHandle(engine.assetManager.shaderDefault2D);
    Shader& debugShader = engine.assetManager.shaders.GetNewObjectAndHandle(engine.assetManager.shaderLightDebug);
    Shader& defaultSkybox = engine.assetManager.shaders.GetNewObjectAndHandle(engine.assetManager.shaderDefaultSkybox);
    Shader& renderTexture = engine.assetManager.shaders.GetNewObjectAndHandle(engine.assetManager.shaderScreenRenderTexture);

    // Assign names
    default3D.SetName("Default3D");
    default2D.SetName("Default2D");
    debugShader.SetName("DebugShader");
    defaultSkybox.SetName(Shader_SkyboxDefault);
    renderTexture.SetName("ScreenRenderTexture");
    // Actually compile them
    default3D.LoadAndCompile();
    default2D.LoadAndCompile();
    debugShader.LoadAndCompile();
    defaultSkybox.LoadAndCompile();
    renderTexture.LoadAndCompile();
#ifdef LOG_DEFAULT_SHADER_COMP
    std::cout << "[shader] compiled " << default3D.GetName() << " " << default3D.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << default2D.GetName() << " " << default2D.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << debugShader.GetName() << " " << debugShader.GetShaderId() << std::endl;
    std::cout << "[shader] compiled " << defaultSkybox.GetName() << " " << defaultSkybox.GetShaderId() << std::endl;
#endif

    default3D.SetAcceptsLighting(true);

    engine.shaderWatcher.WatchShader(default3D);
    engine.shaderWatcher.WatchShader(default2D);
    engine.shaderWatcher.WatchShader(debugShader);
    engine.shaderWatcher.WatchShader(defaultSkybox);
    engine.shaderWatcher.WatchShader(renderTexture);


    // NEW SHADERS
    {
        Handle h {};
        Shader& treeShader = engine.assetManager.shaders.GetNewObjectAndHandle(h);
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

    AssetManager::LoadTextureCubemap(texture, facePaths);
}


void LoadDefaultTextures() {
    EnginePtr->assetManager.CreateDefaultWhiteTexture();
    EnginePtr->assetManager.CreateDefaultNormalMap();
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
}

void EndFrame() {
    SwapBuffers(mainWin.dc);
    EnginePtr->shaderWatcher.ProcessReloads();
}


void UpdateSceneTransforms(GameScene& scene) {
    std::vector<Transform>& allTransforms = scene.transforms.GetVector();
    for (Transform& temp : allTransforms) {
        Transform_UpdateMatrices(temp);
    }
}


void RenderUI() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);
}


void RenderLoop() {
    Engine& eng = *EnginePtr;

    UpdateSceneTransforms(eng.scene);
    StartFrame();
    GL_UpdateBackground(eng.scene);
    GL_RenderScene(eng);

    RenderUI();
    EndFrame();
}
// endregion



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

    int state = Input_IsMouseButtonHeld(GameInputKey::MOUSE_BUTTON_RIGHT) ? 1 : 0;
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
    glm_vec3_scale(worldMove, dt * moveSpeed, worldMove);
    vec3 verticalMove = {0,1,0};
    glm_vec3_scale(verticalMove, dt * verticalShift * moveSpeed, verticalMove);
    glm_vec3_add(worldMove, verticalMove, worldMove);
    glm_vec3_add(cameraTransform.position, worldMove, cameraTransform.position);
}

void ControlSettings() {

    if (Input_IsKeyDown(GameInputKey::KEY_G)) {
        EnginePtr->settings.Gamma_Correction  = !EnginePtr->settings.Gamma_Correction;
        std::cout << "[GammaCorrection]: " << EnginePtr->settings.Gamma_Correction << std::endl;
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

    LoadDefaultShaders(engine);
    LoadDefaultMeshes();
    LoadDefaultTextures();

    GL_InitDefaultMaterials(engine.assetManager);
    InitSkybox(engine.scene.skybox);
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

void SpinWait() {
    while (true) {
        Sleep(100);
    }
}


void CreateEngine() {
    EnginePtr = Engine::GetInstance();
    auto& engine = *EnginePtr;
    InitDefaults();
    engine.UpdateSettings();
    engine.shaderWatcher.WatchSettingsFile();

}

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd) {

    try {
        MakeConsole();

        FetchProjectPath(ProjectSettings::RootPath, ProjectSettings::ResourcesPath);
        printf("-- RootPath %s,  ResourcesPath %s \n", ProjectSettings::RootPath.c_str(), ProjectSettings::ResourcesPath.c_str());

        mainWin.name = "Renderer Window";
        mainWin.width = 1024;
        mainWin.height = 700;
        mainWin.posX = 612;
        mainWin.posY = 100;
        mainWin.hInst = hInstance;
        mainWin.callbackClose = Win_CloseWindow;
        mainWin.callbackResize = Win_Resize;
        mainWin.callbackMove = Win_Move;
        mainWin.callbackResize = Win_Resize;

        bool didInit = CreateFirstWindowAndInitGL(&mainWin);
        if (didInit == false) {
            std::cerr << "FAILED TO LOAD WIN AND GL\n";
            return -10;
        }

        CreateEngine();
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
    }
    // int frames = 0;
    while (!mainWin.close)
    {
        // printf("looping\n");
        try {
            // printf("Frame %d, Delta: %f \n", Time_GetFrameCountInt(), Time_GetDelta());
            Time_Update();
            Win32WindowUpdate(mainWin);
            RenderLoop();
            ControlsLoop();
            Input_Update();
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    std::cout<<"Main Loop terminated. SPIN\n";
    // SpinWait();
    return 0;
}

