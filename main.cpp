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
#include "Slot.h"
#include "ShaderRegistry.h"


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

static float cameraRotationSpeed = 6.0f;
static float cameraMoveSpeed = 5.5f;


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
static GameScene scene {};


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
    for (auto& meshData : obj.meshData) {
        meshData.hShader = s_shaderReg.default3D;
    }
}


void BuildCubeMeshAt(const Handle& handle) {
    Mesh& mesh = s_scene.meshes.GetItemRef(handle);
    Mesh_DefaultCube(mesh);
}
void BuildPyramidMeshAt(const Handle& handle) {
    Mesh& mesh = s_scene.meshes.GetItemRef(handle);
    Mesh_DefaultCube(mesh);
}
void BuildSphereMeshAt(const Handle& handle) {
    Mesh& mesh = s_scene.meshes.GetItemRef(handle);
    Mesh_DefaultCube(mesh);
}
void BuildPlaneMeshAt(const Handle& handle) {
    Mesh& mesh = s_scene.meshes.GetItemRef(handle);
    Mesh_DefaultCube(mesh);
}
void BuildDonutMeshAt(const Handle& handle) {
    Mesh& mesh = s_scene.meshes.GetItemRef(handle);
    Mesh_DefaultCube(mesh);
}
void BuildCapsuleMeshAt(const Handle& handle) {
    Mesh& mesh = s_scene.meshes.GetItemRef(handle);
    Mesh_DefaultCube(mesh);
}





void AllocateGraphicsForObjectGL(RenderObject& obj)
{
    obj.meshData.reserve(1);
    if (obj.meshData.size() < 1) {
        obj.meshData.resize(1);
    }

    for (auto& meshRenderData : obj.meshData) {

        Mesh& mesh = s_scene.meshes.GetItemRef(meshRenderData.hMesh);

        GLuint vao;
        GLuint vbo;
        GLuint ebo;

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
        meshRenderData.vao = vao;
        meshRenderData.vbo = vbo;
        meshRenderData.ebo = ebo;
    }
}

void InitRenderObject1Mesh(Handle& objHandle, const Handle& trHandle, const Handle meshHandle, const Handle shaderHandle) {
    RenderObject& obj = s_scene.renderObjects.GetItemRef(objHandle);
    obj.hTransform = trHandle;
    auto& tr = s_scene.transforms.GetItemRef(trHandle);
    Transform_Init(tr);
    obj.AppendNewMeshAndShader(meshHandle, shaderHandle);
    AllocateGraphicsForObjectGL(obj);
}

void InitRenderObject(Handle& objHandle, Handle& trHandle) {
    auto& tr = s_scene.transforms.GetItemRef(trHandle);
    Transform_Init(tr);
    auto& ro = s_scene.renderObjects.GetItemRef(objHandle);
    ro.hTransform = trHandle;
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
    fbBackground.shaderId = s_shaderReg.GetDefault2D().GetShaderId();

}


void InitShaders() {

    Shader& default3D = s_shaderReg.shaders.MakeNew(s_shaderReg.default3D);
    Shader& default2D = s_shaderReg.shaders.MakeNew(s_shaderReg.default2D);
    default3D.SetName("Default3D");
    default2D.SetName("Default2D");
    bool compiled3D = default3D.Compile();
    bool compiled2D = default2D.Compile();

    if (compiled3D)
        printf("Successfully compiled Default 3D \n");
    if (compiled2D)
        printf("Successfully compiled Default 2D\n");
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
    Camera& camera = s_scene.camera;
    camera.fieldOfView = 60.0f;
    camera.farPlane = 500.0f;
    camera.nearPlane = 0.1f;
    camera.aspectRatio = 1.0f;
    camera.transformHandle = transformHandle;
    Transform& cameraTransform = s_scene.transforms.GetItemRef(transformHandle);
    Transform_Init(cameraTransform);
    Transform_SetLocalPosition(cameraTransform, 0.0f, 1.0f, -6.0f);
    Transform_SetRotationEulerDeg(cameraTransform, 1.0f, 5.0f, 0.0f);
    Transform_SetLocalScale(cameraTransform, 1,1,1);
    LogHandle("CAMERA transform handle", transformHandle);
}


void InitSceneLights(const Handle& transformHandle) {
    s_scene.mainLight.lightType = ELightType::Directional;
    s_scene.mainLight.transformHandle = transformHandle;

    SET_VEC3(s_scene.mainLight.color, 1.0f, 1.0f, 1.0f);
    Transform& lightTransform = s_scene.transforms.GetItemRef(transformHandle);
    Transform_Init(lightTransform);
    Transform_SetRotationEulerDeg(lightTransform, 60, -30 ,0);
    Transform_SetWorldPosition(lightTransform, 0.0f, 10.0f, 0.0f);
    LogHandle("LIGHTS transform handle", transformHandle);

}




Handle AddDefaultCube_PosRotScale(vec3 position, vec3 rotation, vec3 scale) {

    Handle handleRenderObj = s_scene.renderObjects.GetFreeHandle();
    Handle handleMesh = s_scene.meshes.GetFreeHandle();
    Handle handleTransform = s_scene.transforms.GetFreeHandle();

    BuildCubeMeshAt(handleMesh);
    Transform& tr = s_scene.transforms.GetItemRef(handleTransform);
    RenderObject& obj = s_scene.renderObjects.GetItemRef(handleRenderObj);

    InitRenderObject1Mesh(handleRenderObj, handleTransform, handleMesh,  s_shaderReg.default3D);
    obj.SetName("Cube");

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);

    return handleRenderObj;
}

Handle AddDefaultCube_PosRot(vec3 position, vec3 rotation) {
    vec3 scale = {1.0f, 1.0f, 1.0f};
    return AddDefaultCube_PosRotScale(position, rotation, scale);
}


void SetShaderHandlesIfNone() {

    auto& vec = s_scene.renderObjects.GetVector();
    for (auto& temp : vec) {
        if (temp.data.shadersAssigned == false) {
            temp.data.shadersAssigned = true;
            for (auto& meshData : temp.data.meshData) {
                meshData.hShader = s_shaderReg.default3D;
            }
        }
    }
}


Handle LoadTableMesh(vec3 position, vec3 rotation, vec3 scale) {
    Handle objHandle = AssetManager::LoadModel("upd_picnic table.fbx", s_scene);

    RenderObject& ro = s_scene.renderObjects.GetItemRef(objHandle);
    Transform& tr = s_scene.transforms.GetItemRef(ro.hTransform);

    Handle meshH = ro.meshData[0].hMesh;
    printf("-[New]- RO handle {%d, %d}\n", objHandle.index, objHandle.generation);
    printf("-[New]- MESH handle {%d, %d}\n", meshH.index, meshH.generation);

    AllocateGraphicsForObjectGL(ro);

    Transform_SetLocalScaleVec(tr, scale);
    Transform_SetLocalPositionVec(tr, position);
    Transform_SetRotationEulerVec(tr, rotation);

    Mesh& mesh = s_scene.meshes.GetItemRef(ro.meshData[0].hMesh);
    Mesh_Print(mesh);

    ro.SetName("upd_picnic table");
    return objHandle;

}

void InitTestScene() {
    try {
        printf("Init basic scene\n");
        InitShaders();
        InitTextures();

        fbBackground = new FrameBufferUI();
        InitBackgroundQuad(*fbBackground);

        InitCamera(s_scene.transforms.GetFreeHandle());
        InitSceneLights(s_scene.transforms.GetFreeHandle());

        // floor
        {
            vec3 pos = {0.0f, -1.0f, 0.0f};
            vec3 rot = {0.0f, 0.0f, 0.0f};
            vec3 scaleFloor = {25.0f, 2.0f, 25.0f};
            Handle objHandle = AddDefaultCube_PosRotScale(pos, rot, scaleFloor);
            s_scene.existingObjects.push_back(objHandle);
        }
        // cube 1
        {
            vec3 pos = {4.0f, 1.0f, 0.0f};
            vec3 rot = {0.0f, 0.0f, 0.0f};
            vec3 scale = {1.0f, 1.0f, 1.0f};
            Handle objHandle = AddDefaultCube_PosRotScale(pos, rot, scale);
            s_scene.existingObjects.push_back(objHandle);
        }
        // cube 2
        {
            vec3 pos = {-4.0f, 1.0f, 0.0f};
            vec3 rot = {0.0f, 0.0f, 0.0f};
            vec3 scale = {1.0f, 1.0f, 1.0f};
            Handle objHandle = AddDefaultCube_PosRotScale(pos, rot, scale);
            s_scene.existingObjects.push_back(objHandle);
        }
        // cube 3
        {
            vec3 pos = {0.0f, 1.0f, 4.0f};
            vec3 rot = {0.0f, 0.0f, 0.0f};
            vec3 scale = {1.0f, 2.0f, 1.0f};
            Handle objHandle = AddDefaultCube_PosRotScale(pos, rot, scale);
            s_scene.existingObjects.push_back(objHandle);
        }
        // table
        {
            vec3 pos = {0.0f, 1.0f, 0.0f};
            vec3 rot = {-90.0f, 0.0f, 0.0f};
            vec3 scale = {0.01f, 0.01f, 0.01f};
            Handle objHandle = LoadTableMesh(pos, rot, scale);
            s_scene.existingObjects.push_back(objHandle);
        }
        SetShaderHandlesIfNone();
    }
    catch (std::exception& e) {
        std::cerr << "Exception: " << e.what() << std::endl;
    }

}


// region Loops
void StartFrame() {
    vec4& backgroundColor = s_scene.backgroundColor;
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
    Camera& camera = s_scene.camera;

    camera.aspectRatio = (f32)mainWin.width / (f32)mainWin.height;
    Transform& cameraTransform = s_scene.transforms.GetItemRef(camera.transformHandle);
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
    Camera& camera = s_scene.camera;

    mat4 viewProjMatrix;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProjMatrix);

    s_scene.transforms.GetItemRef(s_scene.mainLight.transformHandle);

    std::vector<Slot<Transform>>& allTransforms = s_scene.transforms.GetVector();

    for (Slot<Transform>& temp : allTransforms) {
        TransformUpdate(temp.data);
    }

    Transform& cameraTransform = s_scene.transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = s_scene.transforms.GetItemRef(s_scene.mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);

    auto& shader = s_shaderReg.GetDefault3D();
    glUseProgram(shader.GetShaderId());
    shader.setVec3(ID_UNIFORM_VIEW_POS, cameraTransform.position);
    shader.setVec3(ID_UNIFORM_AMBIENT_LIGHT_COLOR, s_scene.ambientLightColor);
    shader.setFloat(ID_UNIFORM_AMBIENT_LIGHT_INTENSITY, s_scene.ambientIntensity);
    shader.setVec3("DIRECTIONAL_LIGHT.direction", mainLightDir);
    shader.setVec3("DIRECTIONAL_LIGHT.color", s_scene.mainLight.color);
    shader.setFloat("DIRECTIONAL_LIGHT.intensity", s_scene.mainLight.intensity);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    for (auto& objHandle : s_scene.existingObjects) {
        if (s_scene.renderObjects.IsValid(objHandle) == false) {
            continue;
        }
        RenderObject& obj = s_scene.renderObjects.GetItemRef(objHandle);

        Transform& transform = s_scene.transforms.GetItemRef(obj.hTransform);

        // printf("---- Rendering object %s\n", obj.name.c_str());
        // printf("Scale %f\n", transform.scale[0]);
        // printf("Position %f, %f, %f\n", transform.position[0], transform.position[1], transform.position[2]);

        mat4 mvp;
        glm_mat4_mul(viewProjMatrix, transform.modelMatrix, mvp);

        for (auto& renderData : obj.meshData) {

            shader = s_shaderReg.GetShader(renderData.hShader);
            Mesh& mesh = s_scene.meshes.GetItemRef(renderData.hMesh);
            // Mesh_Print(mesh);
            auto shaderId = shader.GetShaderId();

            int model_Location = glGetUniformLocation(shaderId, ID_UNIFORM_MODEL);
            int view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
            int proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);

            glUniformMatrix4fv(model_Location, 1, GL_FALSE, (float*)transform.modelMatrix);
            glUniformMatrix4fv(view_Location, 1, GL_FALSE, (float*)camera.viewMatrix);
            glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (float*)camera.projectionMatrix);

            glUseProgram(shaderId);

            glBindVertexArray(renderData.vao);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            glBindVertexArray(0);
        }
    }
    glBindVertexArray(0);
}


void RenderBackground() {
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_CULL_FACE);

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
    // Animations();

    RenderOpaques();
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

    MakeConsole();

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

    // AssetManager::LoadModel("upd_picnic table.fbx");
    // SpinWait();
    // return -1;

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
    std::cout<<"Main Loop terminated. SPIN\n";
    // SpinWait();
    return 0;
}

