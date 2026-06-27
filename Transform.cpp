#include "Transform.h"

void Transform_Init(Transform& transform) {
    SET_VEC3(transform.position, 0.0f, 0.0f, 0.0f);
    SET_VEC3(transform.scale, 1.0f, 1.0f, 1.0f);
    SET_VEC4(transform.rotation, 0.0f, 0.0f, 0.0f, 1.0f);
}

void SetColor3(vec3 color, float r, float g, float b) {
    color[0] = r;
    color[1] = g;
    color[2] = b;

}
void SetColor4(vec4 color, float r, float g, float b, float a) {
    color[0] = r;
    color[1] = g;
    color[2] = b;
    color[3] = a;
}

void Transform_SetWorldPosition(Transform& transform, float x, float y, float z) {
    transform.position[0] = x;
    transform.position[1] = y;
    transform.position[2] = z;
}

void Transform_SetLocalPosition(Transform& transform, float x, float y, float z) {
    transform.position[0] = x;
    transform.position[1] = y;
    transform.position[2] = z;
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

void GetWorldRotation(Transform& transform, quaternion dest) {
    glm_quat_copy(transform.rotation, dest);
}



void Transform_GetFrw(Transform& transform, vec3 dest) {
    vec3 frw = {0, 0, 1};
    quaternion worldRot;
    GetWorldRotation(transform, worldRot);
    glm_quat_rotatev(worldRot, frw, dest);
}
void Transform_GetBack(Transform& transform, vec3 dest) {
    Transform_GetFrw(transform, dest);
    glm_vec3_scale(dest, -1, dest);
}


void Transform_GetRight(Transform& transform, vec3 dest) {
    vec3 right = {1, 0, 0};
    quaternion worldRot;
    GetWorldRotation(transform, worldRot);
    glm_quat_rotatev(worldRot, right, dest);
}
void Transform_GetLeft(Transform& transform, vec3 dest) {
    Transform_GetRight(transform, dest);
    glm_vec3_scale(dest, -1, dest);
}


void Transform_GetUp(Transform& transform, vec3 dest) {
    vec3 up = {0, 1, 0};
    quaternion worldRot;
    GetWorldRotation(transform, worldRot);
    glm_quat_rotatev(worldRot, up, dest);
}
void Transform_GetDown(Transform& transform, vec3 dest) {
    Transform_GetUp(transform, dest);
    glm_vec3_scale(dest, -1, dest);
}

void Transform_ToWorldVector(Transform& transform, vec3 localVec, vec3 outWorldVec) {
    quaternion worldRot;
    GetWorldRotation(transform, worldRot);
    glm_quat_rotatev(worldRot, localVec, outWorldVec);
}

// Local = POST-MULTIPLY
void Transform_RotateLocalX(Transform& t, float angleDegrees) {
    versor delta;
    vec3 dir = {1, 0 ,0};
    glm_quatv(delta, glm_rad(angleDegrees), dir);
    // POST-MULTIPLY: Current * Delta
    glm_quat_mul(t.rotation, delta, t.rotation);
    glm_quat_normalize(t.rotation);
}
// Local = POST-MULTIPLY
void Transform_RotateLocalY(Transform& t, float angleDegrees) {
    versor delta;
    vec3 dir = {0, 1 ,0};
    glm_quatv(delta, glm_rad(angleDegrees), dir);
    glm_quat_mul(t.rotation, delta, t.rotation);
    glm_quat_normalize(t.rotation);
}
// Local = POST-MULTIPLY
void Transform_RotateLocalZ(Transform& t, float angleDegrees) {
    versor delta;
    vec3 dir = {0, 0 ,1};
    glm_quatv(delta, glm_rad(angleDegrees), dir);
    glm_quat_mul(t.rotation, delta, t.rotation);
    glm_quat_normalize(t.rotation);
}


// World = Pre-MULTIPLY
void Transform_RotateWorldX(Transform& t, float angleDegrees) {
    versor delta;
    vec3 dir = {1, 0, 0};
    glm_quatv(delta, glm_rad(angleDegrees), dir);
    glm_quat_mul(delta, t.rotation, t.rotation);
    glm_quat_normalize(t.rotation);
}

void Transform_RotateWorldY(Transform& t, float angleDegrees) {
    versor delta;
    vec3 dir = {0, 1, 0};
    glm_quatv(delta, glm_rad(angleDegrees), dir);
    glm_quat_mul(delta, t.rotation, t.rotation);
    glm_quat_normalize(t.rotation);
}

void Transform_RotateWorldZ(Transform& t, float angleDegrees) {
    versor delta;
    vec3 dir = {0, 0, 1};
    glm_quatv(delta, glm_rad(angleDegrees), dir);
    glm_quat_mul(delta, t.rotation, t.rotation);
    glm_quat_normalize(t.rotation);
}



void Transform_QuatToEuler(quaternion quat, vec3 outEulers) {
    mat4 rot_matrix;
    glm_quat_mat4(quat, rot_matrix);
    glm_euler_angles(rot_matrix, outEulers);
    if (outEulers[0] != 0)
        outEulers[0] = glm_deg(outEulers[0]);
    if (outEulers[1] != 0)
        outEulers[1] = glm_deg(outEulers[1]);
    if (outEulers[2] != 0)
        outEulers[2] = glm_deg(outEulers[2]);

}