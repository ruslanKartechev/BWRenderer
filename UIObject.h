#pragma once
#include "RenderSubMesh.h"
#include "SlotsMap.h"
#include "cglm/cglm.h"

class UIObject {

public:
    Handle hMaterial;
    vec2 position;
    vec2 size;
    float rotation;

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

};

