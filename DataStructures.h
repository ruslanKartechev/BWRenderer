#pragma once
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


enum class ELightType {
    Directional,
    Point,
    Spot
};


typedef struct {
    size_t handle;
    size_t transformHandle;
    ELightType lightType;
    vec3 color;
    float intensity;
    float angleInner;
    float angleOuter;
} Light;




typedef struct Mesh{
    float* vertexData = nullptr;
    int* indexData = nullptr;

    size_t handle;
    int vertexDataCount;
    int indexCount;
    int stride;

    int startIndexVertex = 0;
    int startIndexUV = 0;
    int startIndexColor = 0;
    int startIndexNormals = 0;

} Mesh;

typedef struct {
    int transformHandle;
    int meshHandle;
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
    size_t transformHandle;
} Camera;

