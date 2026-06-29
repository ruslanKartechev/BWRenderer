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
#include "Slot.h"

enum class ELightType {
    Directional,
    Point,
    Spot
};


typedef struct {
    Handle handle;
    Handle transformHandle;
    ELightType lightType;
    vec3 color;
    float intensity;
    float angleInner;
    float angleOuter;
} Light;





typedef struct {
    Handle transformHandle;
    Handle meshHandle;
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
    Handle transformHandle;
} Camera;

