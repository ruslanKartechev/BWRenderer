#pragma once
#include "SlotsMap.h"

enum class ELightType {
    Directional,
    Point,
    Spot
};

struct Light{
    Handle handle;
    Handle transformHandle;
    ELightType lightType;
    vec3 color;
    float intensity;
    float angleInner;
    float angleOuter;
};



struct Camera{
    float fieldOfView;
    float aspectRatio;
    float nearPlane;
    float farPlane;

    mat4 viewMatrix;
    mat4 projectionMatrix;
    Handle transformHandle;
};




