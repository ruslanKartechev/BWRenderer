#include "Transform.h"

void Transform_Init(Transform& transform) {
    SET_VEC3(transform.position, 0.0f, 0.0f, 0.0f);
    SET_VEC3(transform.scale, 1.0f, 1.0f, 1.0f);
    SET_VEC4(transform.rotation, 0.0f, 0.0f, 0.0f, 1.0f);
}

void Transform_SetRotationEulerDeg(
    Transform& transform,
    float x,
    float y,
    float z)
{
    vec3 eulers;
    eulers[0] = glm_rad(x);
    eulers[1] = glm_rad(y);
    eulers[2] = glm_rad(z);
    glm_euler_xyz_quat(eulers, transform.rotation);
}

void Transform_SetRotationEulerRad(
    Transform& transform,
    float x,
    float y,
    float z)
{
    vec3 eulers;
    eulers[0] = x;
    eulers[1] = y;
    eulers[2] = z;
    glm_euler_xyz_quat(eulers, transform.rotation);
}


void RotateLocalY(Transform& transform, float deltaDeg) {
    vec3 localUp = {0.0f, 1.0f, 0.0f};
    versor deltaRotation;
    glm_quatv(deltaRotation, glm_rad(deltaDeg), localUp);
    glm_quat_mul(transform.rotation, deltaRotation, transform.rotation);
    glm_quat_normalize(transform.rotation);
}
void RotateLocalX(Transform& transform, float deltaDeg) {
    vec3 localUp = {1.0f, 0.0f, 0.0f};
    versor deltaRotation;
    glm_quatv(deltaRotation, glm_rad(deltaDeg), localUp);
    glm_quat_mul(transform.rotation, deltaRotation, transform.rotation);
}
void RotateLocalZ(Transform& transform, float deltaDeg) {
    vec3 localUp = {0.0f, 0.0f, 1.0f};
    versor deltaRotation;
    glm_quatv(deltaRotation, glm_rad(deltaDeg), localUp);
    glm_quat_mul(transform.rotation, deltaRotation, transform.rotation);
}
