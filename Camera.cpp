//
// Created by user on 7/12/2026.
//
#include "Camera.h"
#include "cglm/clipspace/persp_lh_no.h"

Camera::Camera() {
    fieldOfView = 60.0f;
    farPlane = 500.0f;
    nearPlane = 0.1f;
    aspectRatio = 1.0f;
}


void Camera::Init(float fov, float aspectRatio, float nearPlane, float farPlane) {
    this->fieldOfView = fov;
    this->aspectRatio = aspectRatio;
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;

}

void Camera::UpdateNearPlane(float nearPlane) {
    this->nearPlane = nearPlane;
}

void Camera::UpdateFarPlane(float farPlane) {
    this->farPlane = farPlane;

}

void Camera::UpdateNearAndFarPlane(float nearPlane, float farPlane) {
    this->nearPlane = nearPlane;
    this->farPlane = farPlane;
}

void Camera::UpdateMatrices(Transform& transform)
{
    glm_mat4_copy(transform.modelMatrix, viewMatrix);
    glm_inv_tr(viewMatrix);

    glm_perspective_lh_no(glm_rad(fieldOfView),
        aspectRatio,
        nearPlane,
        farPlane,
        projectionMatrix);
}

void Camera::UpdateAspectRatio(float aspectRatio) {
    this->aspectRatio = aspectRatio;
}

void Camera::UpdateAspectRationWidthHeight(float width, float height) {
    if (width > 0 && height > 0)
        this->aspectRatio = width / height;
    else
        this->aspectRatio = 1;
}
