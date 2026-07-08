#include <iostream>
#include <vector>
#include <glad/glad.h>
#include <filesystem>
#include "InputSystem.h"
#include "Application.h"
#include "PlatformWin32.h"
#include "Shader.h"
#include "Texture.h"
#include "GameTime.h"
#include "Transform.h"
#include "GameScene.h"

#include "DataStructures.h"
#include "cglm/cglm.h"
#include "cglm/clipspace/persp_lh_no.h"

#include "AssetManager.h"
#include "RenderObject.h"
#include "SlotsMap.h"
#include "ShaderRegistry.h"
#include "GraphicsGL.h"


#define LOG(str) do {printf("%s\n", str);}while(false);
#define LOG2(str1, str2) do {printf("%s1 %s2\n", str1, str2); }while(false);
#define LOG3(str1, str2, str3) do {printf("%s1 %s2 %s3\n", str1, str2, str3); }while(false);

#define STR_VEC2(vec) "[" << vec[0]<< ", " << vec[1] << "]";
#define STR_VEC3(vec) "[" << vec[0]<< ", " << vec[1] << ", " << vec[2] << "]";
#define STR_VEC4(vec) "[" << vec[0]<< ", " << vec[1] << ", " << vec[2] << ", " << vec[3] << "]";


const char* ID_UNIFORM_MODEL = "MATRIX_MODEL";
const char* ID_UNIFORM_VIEW = "MATRIX_VIEW";
const char* ID_UNIFORM_PROJECTION = "MATRIX_PROJECTION";


const char* ID_UNIFORM_TIME = "GLOBAL_TIME";
const char* ID_UNIFORM_GLOBAL_LIGHT = "GLOBAL_LIGHT";
const char* ID_UNIFORM_VIEW_POS = "VIEW_POS";
const char* ID_UNIFORM_AMBIENT_LIGHT_COLOR = "AMBIENT_LIGHT_COLOR";
const char* ID_UNIFORM_AMBIENT_LIGHT_INTENSITY = "AMBIENT_LIGHT_INTENSITY";

static float cameraRotationSpeed = 6.0f;
static float cameraMoveSpeed = 5.5f;

constexpr f32 LightDebugScale = .25f;

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
static Texture backgroundTexture;

static ShaderRegistry s_shaderReg;
static GameScene s_scene;

static FrameBufferUI* fbBackground = nullptr;
static FrameBufferUI* fbUI = nullptr;


void InitMaterial(Material& material) {
    material.didInit = true;
    

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
    fbBackground.shaderId = s_shaderReg.GetDefault2D().GetShaderId();

}


void InitShaders() {

    Shader& default3D = s_shaderReg.shaders.GetNewObjectAndHandle(s_shaderReg.default3D);
    Shader& default2D = s_shaderReg.shaders.GetNewObjectAndHandle(s_shaderReg.default2D);
    Shader& debugShader = s_shaderReg.shaders.GetNewObjectAndHandle(s_shaderReg.debugShader);
    auto* shaderptr3D = &default3D;
    auto* shaderptr2D = &default2D;
    auto* debugShaderPTR = &debugShader;

    default3D.SetName("Default3D");
    default2D.SetName("Default2D");
    debugShader.SetName("DebugShader");
    bool compiled3D = default3D.LoadAndCompile() == 0;
    bool compiled2D = default2D.LoadAndCompile() == 0;
    bool compiledDebug = debugShader.LoadAndCompile() == 0;

    printf("---> Compiled 3D %d \n", compiled3D);
    printf("---> Compiled 2D %d \n", compiled2D);
    printf("---> Compiled debug %d \n", compiledDebug);

}



void InitTextures() {
    backgroundTexture = Texture();
    if (backgroundTexture.LoadTexture("back1.jpg")) {
        glGenTextures(1, &backgroundTexture.glHandle);
        backgroundTexture.UploadToGL(true);
    }
    else {
        std::cerr << "Failed to load texture! \n";
    }
}




Handle CreateObjectWithCustomMesh(const char* path, vec3 position, vec3 rotation, vec3 scale, Handle shaderHandle) {
    GameScene& scene = s_scene;
    Handle objHandle = AssetManager::LoadModel(path, scene);

    RenderObject& obj = scene.renderObjects.GetItemRef(objHandle);
    Transform& tr = scene.transforms.GetItemRef(obj.hTransform);

    Handle meshH = obj.meshData[0].hMesh;
    printf("-[New]- Transform {%d, %d}\n", obj.hTransform.index, obj.hTransform.generation);
    printf("-[New]- RO handle {%d, %d}\n", objHandle.index, objHandle.generation);
    printf("-[New]- MESH handle {%d, %d}\n", meshH.index, meshH.generation);

    obj.shadersAssigned = true;
    for (auto& subMesh : obj.meshData) {
        subMesh.hShader = shaderHandle;
    }
    // obj.AppendNewMeshAndShader(handleMesh, shaderHandle);
    AllocateGraphicsForObject(obj, scene);

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);
    obj.SetName(path);

    // Mesh& mesh = scene.meshes.GetItemRef(ro.meshData[0].hMesh);
    // Mesh_Print(mesh);
    return objHandle;
}


void SetShaderHandlesIfNone() {

    auto& vec = s_scene.renderObjects.GetVector();
    for (auto& shader : vec) {
        if (shader.shadersAssigned == false) {
            shader.shadersAssigned = true;
            for (auto& meshData : shader.meshData) {
                meshData.hShader = s_shaderReg.default3D;
            }
        }
    }
}



void InitCamera() {
    Camera& camera = s_scene.camera;
    camera.fieldOfView = 60.0f;
    camera.farPlane = 500.0f;
    camera.nearPlane = 0.1f;
    camera.aspectRatio = 1.0f;
    Transform& cameraTransform = s_scene.transforms.GetNewObjectAndHandle(camera.transformHandle);
    Transform_Init(cameraTransform);
    Transform_SetWorldPosition(cameraTransform, 0.0f, 1.0f, -10.0f);
    Transform_SetRotationEulerDeg(cameraTransform, 1.0f, 0.0f, 0.0f);
    Transform_SetLocalScale(cameraTransform, 1,1,1);
}



void InitSceneLights(){
    Light& light = s_scene.mainLight;
    Transform& lightTransform = s_scene.transforms.GetNewObjectAndHandle(light.transformHandle);
    light.lightType = ELightType::Directional;
    light.intensity = 1.2f;
    SET_VEC3(light.color, 1.0f, 1.0f, 1.0f);

    vec3 pos = {0.0f, 4.0f, -4.0f};
    vec3 eulers = {45.0f, 0.0f, 0.0f};
    vec3 scale = {LightDebugScale, LightDebugScale, LightDebugScale};
    Transform_Init(lightTransform);
    Transform_SetLocalPositionRotationScale(lightTransform, pos, eulers, scale);

}

void AddDebugGeometryForLights() {
    {
        vec3 pos, eulers, scale;
        Handle objHandle = s_scene.NewObject_CutConeNamed("Light Debug", pos, eulers, scale, s_shaderReg.debugShader);
        s_scene.existingObjects.push_back(objHandle);
        auto& newObj = s_scene.renderObjects.GetItemRef(objHandle);
        s_scene.transforms.FreeHandle(newObj.hTransform);

        newObj.hTransform = s_scene.mainLight.transformHandle;
        printf("SET debug transform %d, %d\n\n", newObj.hTransform.index, newObj.hTransform.generation);
    }
}

void PlaceObjectsToScene() {
    GameScene& scene = s_scene;
    // cube RR
    {
        vec3 pos = {4.0f, 0.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube RR", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // cube RL
    {
        vec3 pos = {-4.0f, 0.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube RL", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // cube FR
    {
        vec3 pos = {4.0f, 0.5f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube FR", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // cube FL
    {
        vec3 pos = {-4.0f, 1.0f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 2.0f, 1.0f};
        Handle objHandle = scene.NewObject_CubeNamed("Cube FL", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }

    // Sphere RR
    {
        vec3 pos = {-4.0f, 1.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere RL
    {
        vec3 pos = {4.0f, 1.5f, -2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere FR
    {
        vec3 pos = {-4.0f, 2.5f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // Sphere FL
    {
        vec3 pos = {4.0f, 2.5f, 2.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_SphereNamed("Sphere", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }

    // Capsule
    {
        vec3 pos = {10.0f, 1.0f, -5.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_Capsule(pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // Pyramid
    {
        vec3 pos = {0.0f, 2.5f, 0.0f};
        vec3 rot = {0.0f, 0.0f, 0.0f};
        vec3 scale = {1.0f, 1.0f, 1.0f};
        Handle objHandle = scene.NewObject_Pyramid(pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    // Table
    {
        vec3 pos = {0.0f, 1.0f, 0.0f};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {0.006f, 0.006f, 0.006f};
        Handle objHandle = CreateObjectWithCustomMesh("upd_picnic table.fbx", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
    //Floor
    {
        vec3 pos = {0.0f, -0.1f, 0.0f};
        vec3 rot = {-90.0f, 0.0f, 0.0f};
        vec3 scale = {5.0f, 5.0f, 0.0f};
        Handle objHandle = scene.NewObject_PlaneNamed("FLOOR", pos, rot, scale, s_shaderReg.default3D);
        scene.existingObjects.push_back(objHandle);
    }
}


void InitTestScene() {
    try {
        printf("Init basic scene\n");
        InitShaders();
        InitTextures();

        fbBackground = new FrameBufferUI();
        InitBackground(*fbBackground);

        InitCamera();
        InitSceneLights();
        PlaceObjectsToScene();
        AddDebugGeometryForLights();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }
}


// region Loops
void StartFrame() {
    // vec4& backgroundColor  s_scene.backgroundColor;
    vec4 backgroundColor = {1,1,1,1};
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    Camera& camera = s_scene.camera;

    camera.aspectRatio = (f32)mainWin.width / (f32)mainWin.height;
    Transform& cameraTransform = s_scene.transforms.GetItemRef(camera.transformHandle);
    Transform_UpdateMatrices(cameraTransform);
    glm_mat4_copy(cameraTransform.modelMatrix, camera.viewMatrix);

    glm_inv_tr(camera.viewMatrix);
    glm_perspective_lh_no(glm_rad(camera.fieldOfView),
        camera.aspectRatio,
        camera.nearPlane,
        camera.farPlane,
        camera.projectionMatrix);
}

void EndFrame() {
    SwapBuffers(mainWin.dc);
}

static int DidLogTransforms;

void UpdateSceneTransforms(GameScene& scene) {
    std::vector<Transform>& allTransforms = scene.transforms.GetVector();
    for (Transform& temp : allTransforms) {
        Transform_UpdateMatrices(temp);
    }
}



void ForwardRenderOpaques(GameScene& scene, Camera& camera) {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    mat4 viewProjMatrix;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProjMatrix);
    UpdateSceneTransforms(scene);

    scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    Transform& cameraTransform = scene.transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);
    {
        Shader& def3D = s_shaderReg.GetDefault3D();
        glUseProgram(def3D.GetShaderId());
        def3D.setVec3(ID_UNIFORM_VIEW_POS, cameraTransform.position);
        def3D.setVec3(ID_UNIFORM_AMBIENT_LIGHT_COLOR, scene.ambientLightColor);
        def3D.setFloat(ID_UNIFORM_AMBIENT_LIGHT_INTENSITY, scene.ambientIntensity);
        def3D.setVec3("DIRECTIONAL_LIGHT.direction", mainLightDir);
        def3D.setVec3("DIRECTIONAL_LIGHT.color", scene.mainLight.color);
        def3D.setFloat("DIRECTIONAL_LIGHT.intensity", scene.mainLight.intensity);
    }

    for (auto& objHandle : scene.existingObjects) {
        if (scene.renderObjects.IsValid(objHandle) == false) {
            continue;
        }
        RenderObject& obj = scene.renderObjects.GetItemRef(objHandle);
        Transform& transform = scene.transforms.GetItemRef(obj.hTransform);

        if (DidLogTransforms < 2) {
            printf("---- Rendering object %s\n", obj.name.c_str());
            printf("Transform %d, %d, %p\n", obj.hTransform.index, obj.hTransform.generation, &transform);
            printf("Scale %f, %f, %f\n", transform.scale[0], transform.scale[1], transform.scale[2]);
            printf("\n");
            // printf("Scale %f\n", transform.scale[0]);
            // printf("Position %f, %f, %f\n", transform.position[0], transform.position[1], transform.position[2]);
        }

        mat4 mvp;
        glm_mat4_mul(viewProjMatrix, transform.modelMatrix, mvp);

        for (auto& renderData : obj.meshData) {

            Shader& shader = s_shaderReg.GetShader(renderData.hShader);
            bool isNull = s_shaderReg.shaders.IsNullItem(shader);
            if (isNull) {
                printf("IS NULL OBTAINED %d \n", isNull);
                continue;
            }
            Mesh& mesh = scene.meshes.GetItemRef(renderData.hMesh);
            auto shaderId = shader.GetShaderId();
            glUseProgram(shaderId);
            int model_Location = glGetUniformLocation(shaderId, ID_UNIFORM_MODEL);
            int view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
            int proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);

            glUniformMatrix4fv(model_Location, 1, GL_FALSE, (float*)transform.modelMatrix);
            glUniformMatrix4fv(view_Location, 1, GL_FALSE, (float*)camera.viewMatrix);
            glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (float*)camera.projectionMatrix);


            glBindVertexArray(renderData.vao);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
    glBindVertexArray(0);

    DidLogTransforms++;
    if ( DidLogTransforms < 2) {
        printf("\n");
        printf("\n");
        printf("\n");
    }
}


void RenderBackground() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

    if (fbBackground == nullptr) {
        std::cout<<"... NO BACKGROUND\n";
        return;
    }
    // printf("shader id %d\n", fbBackground->shaderId);
    glUseProgram(fbBackground->shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture.glHandle);

    glBindVertexArray(fbBackground->vao);
    glDrawElements(GL_TRIANGLES, fbBackground->idxCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void RenderUI() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

}


void Animations() {
    float dt = Time_GetDelta();
    float rotDelta = 25.0f * dt;
    for(auto& handle : s_scene.existingObjects) {
        // Transform& tr = transforms.GetItemRef(obj.transformHandle);
        // RotateLocalY(tr, rotDelta);
    }
}



void RenderLoop() {
    StartFrame();
    RenderBackground();

    ForwardRenderOpaques(s_scene, s_scene.camera);
    RenderUI();
    EndFrame();


}
// endregion





void GameInputs() {
    float dt = (float)Time_GetDelta();
    Transform& cameraTransform = s_scene.transforms.GetItemRef(s_scene.camera.transformHandle);

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
        glm_vec2_scale(mouseDelta, dt * cameraRotationSpeed, mouseDelta);

        vec3 eulersBefore;
        vec3 eulersAfter;
        Transform_QuatToEuler(cameraTransform.rotation, eulersBefore);

        Transform_RotateWorldY(cameraTransform, mouseDelta[0]);
        Transform_RotateLocalX(cameraTransform, -mouseDelta[1]);
        Transform_QuatToEuler(cameraTransform.rotation, eulersAfter);
    }

    vec3 worldMove;
    Transform_ToWorldVector(cameraTransform, localMove, worldMove);
    glm_vec3_scale(worldMove, dt * cameraMoveSpeed, worldMove);
    vec3 verticalMove = {0,1,0};
    glm_vec3_scale(verticalMove, dt * verticalShift * cameraMoveSpeed, verticalMove);
    glm_vec3_add(worldMove, verticalMove, worldMove);
    glm_vec3_add(cameraTransform.position, worldMove, cameraTransform.position);
}


void GameLoop() {
    GameInputs();
}


// region WinProc callbacks and Inputs
int CloseWindow() {
    mainWin.close = true;
    PostQuitMessage(0);
    return 0;
}


int Move(uint32_t newWidth, uint32_t newHeight) {
    mainWin.posX = newWidth;
    mainWin.posY = newHeight;
    return 0;
}

int Resize(uint32_t newWidth, uint32_t newHeight) {
    mainWin.width = newWidth;
    mainWin.height = newHeight;
    glViewport(0, 0, newWidth, newHeight);
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


int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd) {

    try {
        MakeConsole();

        FetchProjectPath(Application::RootPath, Application::ResourcesPath);
        printf("-- RootPath %s,  ResourcesPath %s \n", Application::RootPath.c_str(), Application::ResourcesPath.c_str());

        mainWin.name = "Renderer Window";
        mainWin.width = 1024;
        mainWin.height = 700;
        mainWin.posX = 612;
        mainWin.posY = 100;
        mainWin.hInst = hInstance;
        mainWin.callbackClose = CloseWindow;
        mainWin.callbackResize = Resize;
        mainWin.callbackMove = Move;
        mainWin.callbackResize = Resize;

        bool didInit = CreateFirstWindowAndInitGL(&mainWin);
        if (didInit == false) {
            std::cerr << "FAILED TO LOAD WIN AND GL\n";
            return -10;
        }

        Time_Init();
        Time_SetTargetFrameRate(60);
        InitTestScene();
    }
    catch(std::exception& e) {
        std::cerr << e.what() << std::endl;
    }

    printf("FIRST TIME LOOP RENDER \n\n");
    int i = 0;
    for (const auto& slot : s_shaderReg.shaders.slots) {
        printf("slot at [%d] shader is %s \n\n", i, slot.GetName().c_str());
        i++;
    }

    // int frames = 0;
    while (!mainWin.close)
    {
        // printf("looping\n");
        try {
            // printf("Frame %d, Delta: %f \n", Time_GetFrameCountInt(), Time_GetDelta());
            Time_Update();
            Win32WindowUpdate(mainWin);
            Input_Update();
            RenderLoop();
            GameLoop();
        }
        catch (std::exception& e) {
            std::cerr << e.what() << std::endl;
        }
    }
    std::cout<<"Main Loop terminated. SPIN\n";
    // SpinWait();
    return 0;
}

