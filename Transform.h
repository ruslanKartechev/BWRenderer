#ifndef RENDERGL_TRANSFORM_H
#define RENDERGL_TRANSFORM_H
#include "cglm/cglm.h"
#include "MyTypes.h"

typedef struct {
    mat4 modelMatrix; // updated every frame or so
    versor rotation; // quaternion
    vec3 position;
    vec3 scale;
} Transform;

void Transform_Init(Transform& transform);
void Transform_SetRotationEulerDeg(Transform& transform, float x, float y, float z);
void Transform_SetRotationEulerRad(Transform& transform, float x, float y, float z);


void RotateLocalX(Transform& transform, float deltaDeg);
void RotateLocalY(Transform& transform, float deltaDeg);
void RotateLocalZ(Transform& transform, float deltaDeg);

#endif //RENDERGL_TRANSFORM_H
