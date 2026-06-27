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
#define HANDLE_INDEX(H) (H - 1)

#define LOG(str) do {printf("%s\n", str);}while(false);
#define LOG2(str1, str2) do {printf("%s1 %s2\n", str1, str2); }while(false);
#define LOG3(str1, str2, str3) do {printf("%s1 %s2 %s3\n", str1, str2, str3); }while(false);

const char* ID_UNIFORM_MVP = "MATRIX_MVP";
const char* ID_UNIFORM_TIME = "GLOBAL_TIME";


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


typedef struct Mesh{
    float* vertexData = nullptr;
    int* indexData = nullptr;

    size_t handle;
    int vertexCount;
    int indexCount;
    int stride;

    int startIndexVertex = 0;
    int startIndexUV = 0;
    int startIndexColor = 0;
    int startIndexNormals = 0;

} Mesh;

typedef struct {
    size_t transformHandle;
    size_t meshHandle;
    GLuint shaderId;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
} RenderObject;



typedef struct {
    float fieldOfView;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    mat4 viewMatrix;
    mat4 projectionMatrix;
} Camera;

// endregion


// region Static Data
static WindowParams mainWin = {};
static Texture backgroundTexture;

static FrameBufferUI* fbBackground = nullptr;
static FrameBufferUI* fbUI = nullptr;
static Shader* default2dShader = nullptr;
static Shader* default3dShader = nullptr;
static vec4 backgroundColor = {0.8f, 0.5f, 0.76f, 1.0f};

static Camera camera = {};
static Transform cameraTransform = {};

static Mesh meshes[3] = {Mesh(), Mesh(), Mesh()};
static Transform transforms[3] = {Transform(), Transform(), Transform()};

static RenderObject objects[3] = {RenderObject(), RenderObject(), RenderObject()};

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



void ClearMesh() {
}


void BuildCubeMesh(Mesh& mesh) {
    constexpr float d = 0.5f;
    mesh.vertexCount = 24 * 5;
    mesh.indexCount = 36;
    mesh.stride = 5;
    mesh.startIndexVertex = 0;
    mesh.startIndexUV = 3;
    mesh.startIndexColor = -1;
    mesh.startIndexNormals = -1;
    mesh.vertexData = new float[mesh.vertexCount] {
        // POSITION    // UV
        // Front Face (Z = 0.5f)
        -d, -d,  d,   0.0f, 0.0f,
         d, -d,  d,   1.0f, 0.0f,
         d,  d,  d,   1.0f, 1.0f,
        -d,  d,  d,   0.0f, 1.0f,
        // Back Face (Z = -d)
         d, -d, -d,   0.0f, 0.0f,
        -d, -d, -d,   1.0f, 0.0f,
        -d,  d, -d,   1.0f, 1.0f,
         d,  d, -d,   0.0f, 1.0f,
        // Left Face (X = -d)
        -d, -d, -d,   0.0f, 0.0f,
        -d, -d,  d,   1.0f, 0.0f,
        -d,  d,  d,   1.0f, 1.0f,
        -d,  d, -d,   0.0f, 1.0f,
        // Right Face (X = d)
         d, -d,  d,   0.0f, 0.0f,
         d, -d, -d,   1.0f, 0.0f,
         d,  d, -d,   1.0f, 1.0f,
         d,  d,  d,   0.0f, 1.0f,
        // Top Face (Y = d)
        -d,  d,  d,   0.0f, 0.0f,
         d,  d,  d,   1.0f, 0.0f,
         d,  d, -d,   1.0f, 1.0f,
        -d,  d, -d,   0.0f, 1.0f,
        // Bottom Face (Y = -d)
        -d, -d, -d,   0.0f, 0.0f,
         d, -d, -d,   1.0f, 0.0f,
         d, -d,  d,   1.0f, 1.0f,
        -d, -d,  d,   0.0f, 1.0f
    };
    mesh.indexData = new int[mesh.indexCount] {
        0,  1,  2,    2,  3,  0,  // Front Face
        4,  5,  6,    6,  7,  4,  // Back Face
        8,  9,  10,   10, 11, 8,  // Left Face
        12, 13, 14,   14, 15, 12, // Right Face
        16, 17, 18,   18, 19, 16, // Top Face
        20, 21, 22,   22, 23, 20  // Bottom Face
    };
}


void InitMaterial(RenderObject& obj) {
    obj.shaderId = default3dShader->GetShaderID();

}

void AddRenderObj(RenderObject& obj,
    const Mesh& mesh,
    const size_t transformHandle) {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    obj.transformHandle = transformHandle;
    obj.meshHandle = mesh.handle;
    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);

    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertexCount, mesh.vertexData, GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(float)*mesh.indexCount, mesh.indexData, GL_DYNAMIC_DRAW);

    int stride = mesh.stride * sizeof(float);
    int idx = 0;
    glVertexAttribPointer(idx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)0); // XYZ
    glEnableVertexAttribArray(idx);
    idx++;
    if (mesh.startIndexUV >= 0) {
        glVertexAttribPointer(idx, 2, GL_FLOAT, GL_TRUE, stride, (const void*)(mesh.startIndexUV*sizeof(float))); // UV
        glEnableVertexAttribArray(idx);
        idx++;
    }
    if (mesh.startIndexColor >= 0) {
        glVertexAttribPointer(idx, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexColor*sizeof(float))); // RGBA
        glEnableVertexAttribArray(idx);
        idx++;
    }
    if (mesh.startIndexNormals >= 0) {
        glVertexAttribPointer(idx, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexColor*sizeof(float))); // RGBA
        glEnableVertexAttribArray(idx);
        idx++;
    }

    glBindVertexArray(0);
    obj.vao = vao;
    obj.vbo = vbo;
    obj.ebo = ebo;
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
        0, 2, 1, // First Triangle
        0, 3, 2  // Second Triangle
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

void InitCamera() {
    camera.fieldOfView = 60.0f;
    camera.farPlane = 500.0f;
    camera.nearPlane = 0.1f;
    camera.aspectRatio = 1.0f;
    Transform_Init(cameraTransform);
    SET_VEC3(cameraTransform.position, 0.0f, 0.25f, +5.0f);

    Transform_SetRotationEulerDeg(cameraTransform, 1.0f, 5.0f, 0.0f);
}

void InitScene() {
    printf("Init basic scene\n");
    InitShaders();
    InitTextures();
    InitCamera();
    fbBackground = new FrameBufferUI();
    InitBackgroundQuad(*fbBackground);

    BuildCubeMesh(meshes[0]);
    BuildCubeMesh(meshes[1]);
    BuildCubeMesh(meshes[2]);

    const int count = 3;
    for (int i = 0; i < count; i++) {
        Transform_Init(transforms[i]);
    }
    for (int i = 0; i < count; i++) {
        meshes[i].handle = i+1;
    }

    for (int i = 0; i < count; i++) {
        Transform_SetRotationEulerDeg(transforms[i],
            0.0f, 30.0f + i * 10, 0.0f);

        SET_VEC3(transforms[i].position, 0.25f, -1 + i*1.25f, 0.0f );
    }
    AddRenderObj(objects[0], meshes[0], 1);
    AddRenderObj(objects[1], meshes[0], 2);
    AddRenderObj(objects[2], meshes[0], 3);

    InitMaterial(objects[0]);
    InitMaterial(objects[1]);
    InitMaterial(objects[2]);
}



// region Loops
void StartFrame() {
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    camera.aspectRatio = (f32)mainWin.width / (f32)mainWin.height;
    TransformUpdate(cameraTransform);
    glm_mat4_copy(cameraTransform.modelMatrix, camera.viewMatrix);
    glm_inv_tr(camera.viewMatrix);
    glm_perspective(glm_rad(camera.fieldOfView),
        camera.aspectRatio,
        camera.nearPlane,
        camera.farPlane,
        camera.projectionMatrix);
    printf("Camera AR: %f (%d, %d)", camera.aspectRatio, mainWin.width, mainWin.height);
}

void EndFrame() {
    SwapBuffers(mainWin.dc);
}

void RenderOpaques() {
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    mat4 viewProjMatrix;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProjMatrix);
    // printf("CAMERA Position %f, %f, %f\n",
    //     cameraTransform.position[0],
    //     cameraTransform.position[1],
    //     cameraTransform.position[2]);

    size_t count = 3;
    for (size_t i = 0; i < count; i ++) {
        RenderObject& obj = objects[i];
        Transform& transform = transforms[HANDLE_INDEX(obj.transformHandle)];
        TransformUpdate (transform);
    }

    for (size_t i = 0; i < count; i ++) {
        RenderObject& obj = objects[i];
        Mesh& mesh = meshes[HANDLE_INDEX(obj.meshHandle)];
        Transform& transform = transforms[HANDLE_INDEX(obj.transformHandle)];
        mat4 mvp;
        glm_mat4_mul(viewProjMatrix, transform.modelMatrix, mvp);

        glUseProgram(obj.shaderId);
        int mvpLocation = glGetUniformLocation(obj.shaderId, ID_UNIFORM_MVP);
        glUniformMatrix4fv(mvpLocation, 1, GL_FALSE, (float*)mvp);

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
    for(auto& obj : objects) {
        Transform& tr = transforms[HANDLE_INDEX(obj.transformHandle)];
        RotateLocalY(tr, rotDelta);
    }

}


void Inputs() {
    if (Input_IsKeyDown(GameInputKey::KEY_W)) {
        printf("key W down\n");
    }
    if (Input_IsKeyDown(GameInputKey::KEY_A)) {
        printf("key A down\n");
    }
    if (Input_IsKeyDown(GameInputKey::KEY_S)) {
        printf("key S down\n");
    }
    if (Input_IsKeyDown(GameInputKey::KEY_D)) {
        printf("key D down\n");
    }
    vec2 mousePosition;
    Input_GetMousePosition(mousePosition);
    printf("Mouse position: %f, %f\n", mousePosition[0], mousePosition[1]);
}

void RenderLoop() {
    Inputs();
    StartFrame();
    RenderBackground();
    Animations();

    RenderOpaques();
    RenderUI();
    EndFrame();
}
// endregion



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
    InitScene();
    // int frames = 0;
    while (!mainWin.close)
    {
        printf("Frame %d, Delta: %f \n", Time_GetFrameCountInt(), Time_GetDelta());
        Time_Update();
        WindowUpdate(mainWin);
        RenderLoop();
    }
    std::cout<<"Main Loop terminated\n";
    return 0;
}

