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


void Transform_SetLocalScaleVec(Transform& transform, vec3 scaleVec);
void Transform_SetLocalScale(Transform& transform, float x, float y, float z);


void Transform_SetRotationEulerVec(Transform& transform, vec3 eulers);
void Transform_SetRotationEulerDeg(Transform& transform, float x, float y, float z);
void Transform_SetRotationEulerRad(Transform& transform, float x, float y, float z);


void SetColor3(vec3 color, float r, float g, float b);
void SetColor4(vec4 color, float r, float g, float b, float a);

void Transform_SetWorldPosition(Transform& transform, float x, float y, float z);
void Transform_SetLocalPosition(Transform& transform, float x, float y, float z);
void Transform_SetLocalPositionVec(Transform& transform, vec3 position);

void Transform_GetFrw(Transform& transform, vec3 dest);
void Transform_GetBack(Transform& transform, vec3 dest);

void Transform_GetRight(Transform& transform, vec3 dest);
void Transform_GetLeft(Transform& transform, vec3 dest);

void Transform_GetUp(Transform& transform, vec3 dest);
void Transform_GetDown(Transform& transform, vec3 dest);

void RotateLocalX(Transform& transform, float deltaDeg);
void RotateLocalY(Transform& transform, float deltaDeg);
void RotateLocalZ(Transform& transform, float deltaDeg);

void Transform_ToWorldVector(Transform& transform, vec3 localVec, vec3 outWorldVec);

void GetWorldRotation(Transform& transform, quaternion dest);

void Transform_RotateLocalX(Transform& t, float angleDegrees);
void Transform_RotateLocalY(Transform& t, float angleDegrees);
void Transform_RotateLocalZ(Transform& t, float angleDegrees);

void Transform_RotateWorldX(Transform& t, float angleDegrees);
void Transform_RotateWorldY(Transform& t, float angleDegrees);
void Transform_RotateWorldZ(Transform& t, float angleDegrees);

void Transform_QuatToEuler(quaternion quat, vec3 outEulers);

#endif //RENDERGL_TRANSFORM_H
