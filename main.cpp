#include <iostream>
#include <glad/glad.h>
#include <filesystem>
#include "InputSystem.h"
#include "cglm/cglm.h"
#include "Application.h"
#include "PlatformWin32.h"
#include "Shader.h"
#include "Texture.h"
#include "GameTime.h"
#include "Transform.h"
#include "cglm/clipspace/persp_lh_no.h"
#include "DataStructures.h"
#include <vector>
#include "Slot.h"
#include "Mesh.h"

#define HANDLE_INDEX(H) (H - 1)

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

static FrameBufferUI* fbBackground = nullptr;
static FrameBufferUI* fbUI = nullptr;
static Shader* default2dShader = nullptr;
static Shader* default3dShader = nullptr;
static vec4 backgroundColor = {0.8f, 0.5f, 0.76f, 1.0f};

static std::vector<Handle> sceneObjectHandles = {};


static SlotMap<Mesh> meshes = {};
static SlotMap<Transform> transforms = {};
static SlotMap<RenderObject> renderObjects = {};

static Camera camera = {};
static Light mainLight;

vec3 ambientLightColor = {0.4f, 1.0f, 1.0f};
float ambientIntensity = 0.2;

void LogHandle(const char* msg, const Handle& handle) {

    printf(msg);
    printf("Handle(%d, %d)", handle.index, handle.generation);
    printf("\n");
}

void ReserveSpace() {
}
// endregion


// region Utils
void Transform_SetRotationEulerDeg(Transform& transform, vec3 angles) {
    mat4 matrix;
    glm_euler_xyz(angles, matrix);
    glm_mat4_quat(matrix, transform.rotation);
}

void TransformUpdate(Transform& transform) {
    glm_mat4_identity(transform.modelMatrix);
    glm_translate(transform.modelMatrix, transform.position);
    mat4 rotMat;
    glm_quat_mat4(transform.rotation, rotMat);
    glm_mat4_mul(transform.modelMatrix, rotMat, transform.modelMatrix);
    glm_scale(transform.modelMatrix, transform.scale);
}
//endregion




void InitMaterial(RenderObject& obj) {
    obj.shaderId = default3dShader->GetShaderID();

}


void BuildCubeMeshWithHandle(const Handle& handle) {
    Mesh& mesh = meshes.GetItemRef(handle);
    BuildCubeMesh(mesh);
}


void AddRenderObj(const Handle& renderObjHandle,
    const Handle& meshHandle,
    const Handle& transformHandle)
{
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    RenderObject& obj = renderObjects.GetItemRef(renderObjHandle);
    obj.transformHandle = transformHandle;
    obj.meshHandle = meshHandle;
    Mesh& mesh = meshes.GetItemRef(meshHandle);
    Transform& transform = transforms.GetItemRef(transformHandle);
    Transform_Init(transform);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertexDataCount, mesh.vertexData, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * mesh.indexCount, mesh.indexData, GL_DYNAMIC_DRAW);

    int stride = mesh.stride * sizeof(float);
    int attributeIdx = 0;
    glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)0); // XYZ
    glEnableVertexAttribArray(attributeIdx);
    attributeIdx++;
    if (mesh.startIndexUV >= 0) {
        glVertexAttribPointer(attributeIdx, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexUV*sizeof(float))); // UV
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }
    if (mesh.startIndexNormals >= 0) {
        glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexNormals*sizeof(float))); // XYZ normals
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }
    if (mesh.startIndexColor >= 0) {
        glVertexAttribPointer(attributeIdx, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexColor*sizeof(float))); // RGBA
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }

    glBindVertexArray(0);
    obj.vao = vao;
    obj.vbo = vbo;
    obj.ebo = ebo;

    sceneObjectHandles.push_back(renderObjHandle);
}

void InitBackgroundQuad(FrameBufferUI& fbBackground) {
    // actual background quad
    float vertices[32] = {
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
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

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
    fbBackground.shaderId = default2dShader->GetShaderID();

}


void InitShaders() {
    default2dShader = new Shader("Default2D");
    default3dShader = new Shader("Default3D");

    bool compiled2D = default2dShader->Compile() == 0;
    bool compiled3D = default3dShader->Compile() == 0;

    if (compiled2D)
        printf("Successfully compiled Default 2D\n");
    if (compiled3D)
        printf("Successfully compiled Default 3D \n");
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

void InitCamera(const Handle& transformHandle) {
    camera.fieldOfView = 60.0f;
    camera.farPlane = 500.0f;
    camera.nearPlane = 0.1f;
    camera.aspectRatio = 1.0f;
    camera.transformHandle = transformHandle;
    Transform& cameraTransform = transforms.GetItemRef(transformHandle);
    Transform_Init(cameraTransform);
    Transform_SetLocalPosition(cameraTransform, 0.0f, 1.0f, -6.0f);
    Transform_SetRotationEulerDeg(cameraTransform, 1.0f, 5.0f, 0.0f);
    Transform_SetLocalScale(cameraTransform, 1,1,1);
    LogHandle("CAMERA transform handle", transformHandle);
}


void InitSceneLights(const Handle& transformHandle) {
    mainLight.lightType = ELightType::Directional;
    mainLight.transformHandle = transformHandle;

    SET_VEC3(mainLight.color, 1.0f, 1.0f, 1.0f);
    Transform& lightTransform = transforms.GetItemRef(transformHandle);
    Transform_Init(lightTransform);
    Transform_SetRotationEulerDeg(lightTransform, -45, -5 ,0);
    Transform_SetWorldPosition(lightTransform, 0.0f, 10.0f, 0.0f);
    LogHandle("LIGHTS transform handle", transformHandle);
}



Handle AddDefaultCube_PosRotScale(vec3 position, vec3 rotation, vec3 scale) {

    auto handleRenderObj = renderObjects.GetFreeHandle();
    auto handleMesh = meshes.GetFreeHandle();
    auto handleTransform = transforms.GetFreeHandle();

    BuildCubeMeshWithHandle(handleMesh);
    AddRenderObj(handleRenderObj, handleMesh, handleTransform);

    Transform& tr = transforms.GetItemRef(handleTransform);
    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);

    InitMaterial(renderObjects.GetItemRef(handleRenderObj));

    return handleRenderObj;
}

Handle AddDefaultCube_PosRot(vec3 position, vec3 rotation) {
    vec3 scale = {1.0f, 1.0f, 1.0f};
    return AddDefaultCube_PosRotScale(position, rotation, scale);
}



void InitTestScene() {
    printf("Init basic scene\n");
    InitShaders();
    InitTextures();

    fbBackground = new FrameBufferUI();
    InitBackgroundQuad(*fbBackground);

    InitCamera(transforms.GetFreeHandle());
    InitSceneLights(transforms.GetFreeHandle());

    vec3 pos1 = {0.0f, -1.0f, 0.0f};
    vec3 rot1 = {0.0f, 0.0f, 0.0f};
    vec3 scale1 = {25.0f, 2.0f, 25.0f};
    AddDefaultCube_PosRotScale(pos1, rot1, scale1);

    vec3 pos2 = {1.5f, 1.0f, 6.0f};
    vec3 rot2 = {0.0f, 0.0f, 0.0f};
    AddDefaultCube_PosRot(pos2, rot2);

    vec3 pos3 = {-1.0f, 1.0f, 2.5f};
    vec3 rot3 = {0.0f, 0.0f, 0.0f};
    AddDefaultCube_PosRot(pos3, rot3);

}


// region Loops
void StartFrame() {
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    camera.aspectRatio = (f32)mainWin.width / (f32)mainWin.height;
    Transform& cameraTransform = transforms.GetItemRef(camera.transformHandle);
    // printf("Camera position [%f, %f, %f] \n", cameraTransform.position[0], cameraTransform.position[1], cameraTransform.position[2]);
    TransformUpdate(cameraTransform);
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

void RenderOpaques() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    mat4 viewProjMatrix;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProjMatrix);

    transforms.GetItemRef(mainLight.transformHandle);

    std::vector<Slot<Transform>>& allTransforms = transforms.GetVector();

    for (Slot<Transform>& temp : allTransforms) {
        TransformUpdate(temp.data);
    }

    Transform& cameraTransform = transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = transforms.GetItemRef(mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);

    glUseProgram(default3dShader->GetShaderID());

    default3dShader->setVec3(ID_UNIFORM_VIEW_POS, cameraTransform.position);
    default3dShader->setVec3(ID_UNIFORM_AMBIENT_LIGHT_COLOR, ambientLightColor);
    default3dShader->setFloat(ID_UNIFORM_AMBIENT_LIGHT_INTENSITY, ambientIntensity);

    default3dShader->setVec3("DIRECTIONAL_LIGHT.direction", mainLightDir);
    default3dShader->setVec3("DIRECTIONAL_LIGHT.color", mainLight.color);
    default3dShader->setFloat("DIRECTIONAL_LIGHT.intensity", mainLight.intensity);

    for (auto& objHandle : sceneObjectHandles) {

        if (renderObjects.IsValid(objHandle) == false) {
            continue;
        }
        RenderObject& obj = renderObjects.GetItemRef(objHandle);
        Mesh& mesh = meshes.GetItemRef(obj.meshHandle);
        Transform& transform = transforms.GetItemRef(obj.transformHandle);

        // LogHandle("rendering ", objHandle);
        // LogHandle("mesh ", obj.meshHandle);
        // LogHandle("transform ", obj.transformHandle);
        // printf("opaque pos [%f, %f, %f] \n", transform.position[0], transform.position[1], transform.position[2]);
        // printf("opaque scale [%f, %f, %f] \n", transform.scale[0], transform.scale[1], transform.scale[2]);
        // printf("opaque rotation quat [%f, %f, %f, %f] \n", transform.rotation[0], transform.rotation[1], transform.rotation[2], transform.rotation[3]);

        mat4 mvp;
        glm_mat4_mul(viewProjMatrix, transform.modelMatrix, mvp);

        glUseProgram(obj.shaderId);

        int model_Location = glGetUniformLocation(obj.shaderId, ID_UNIFORM_MODEL);
        int view_Location = glGetUniformLocation(obj.shaderId, ID_UNIFORM_VIEW);
        int proj_Location = glGetUniformLocation(obj.shaderId, ID_UNIFORM_PROJECTION);

        glUniformMatrix4fv(model_Location, 1, GL_FALSE, (float*)transform.modelMatrix);
        glUniformMatrix4fv(view_Location, 1, GL_FALSE, (float*)camera.viewMatrix);
        glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (float*)camera.projectionMatrix);

        glBindVertexArray(obj.vao);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
        glBindVertexArray(0);
    }
    glBindVertexArray(0);
}

void RenderBackground() {
    glDisable(GL_DEPTH_TEST);

    if (fbBackground == nullptr) {
        std::cout<<"nullptr\n";
        return;
    }

    glUseProgram(fbBackground->shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, backgroundTexture.glHandle);

    glBindVertexArray(fbBackground->vao);
    glDrawElements(GL_TRIANGLES, fbBackground->idxCount, GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void RenderUI() {
    glDisable(GL_DEPTH_TEST);
}


void Animations() {
    float dt = Time_GetDelta();
    float rotDelta = 25.0f * dt;
    for(auto& handle : sceneObjectHandles) {

        // Transform& tr = transforms.GetItemRef(obj.transformHandle);
        // RotateLocalY(tr, rotDelta);
    }
}



void RenderLoop() {
    StartFrame();
    RenderBackground();
    // Animations();

    RenderOpaques();
    RenderUI();
    EndFrame();
}
// endregion



static float cameraRotationSpeed = 3.0f;
static float cameraMoveSpeed = 2.5f;


void GameInputs() {
    float dt = (float)Time_GetDelta();
    Transform& cameraTransform = transforms.GetItemRef(camera.transformHandle);

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



int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE prevInstance, LPSTR lpCmdLine, int nShowCmd) {
    FetchProjectPath(Application::RootPath, Application::ResourcesPath);
    printf("-- RootPath %s,  ResourcesPath %s \n", Application::RootPath.c_str(), Application::ResourcesPath.c_str());

    mainWin.name = "Renderer Window";
    mainWin.width = 800;
    mainWin.height = 600;
    mainWin.posX = 512;
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
    ReserveSpace();
    InitTestScene();
    // int frames = 0;
    while (!mainWin.close)
    {
        // printf("Frame %d, Delta: %f \n", Time_GetFrameCountInt(), Time_GetDelta());
        Time_Update();
        Win32WindowUpdate(mainWin);
        Input_Update();
        RenderLoop();
        GameLoop();
    }
    std::cout<<"Main Loop terminated\n";
    return 0;
}

