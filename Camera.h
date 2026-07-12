#pragma once
#include <cglm/cglm.h>
#include "SlotsMap.h"
#include "Transform.h"


class Camera {
public:
    /// Field of View in Degrees
    float fieldOfView;
    // Width over Height
    float aspectRatio;
    float nearPlane;
    float farPlane;

    mat4 viewMatrix;
    mat4 projectionMatrix;
    // Handle to a transform component
    Handle transformHandle;

    Camera();

    void UpdateAspectRatio(float aspectRatio);
    void UpdateAspectRationWidthHeight(float width, float height);;

    void Init(float fov, float aspectRatio, float nearPlane, float farPlane);

    void UpdateNearPlane(float nearPlane);

    void UpdateFarPlane(float farPlane);

    void UpdateNearAndFarPlane(float nearPlane, float farPlane);

    void UpdateMatrices(Transform& transform);
};
