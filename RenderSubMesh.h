#pragma once
#include "SlotsMap.h"
#include "glad/glad.h"


struct RenderSubMesh {
    Handle hMesh;
    Handle hMaterial;
    GLuint vao;
    GLuint vbo;
    GLuint ebo;
};
