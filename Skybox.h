#pragma once
#include "RenderSubMesh.h"
#include "glad/glad.h"

struct Skybox {
    RenderSubMesh renderData;
    GLuint textureCubeMap;
};
