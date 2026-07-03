#pragma once
#include "Shader.h"
#include "Slot.h"


class ShaderRegistry{
public:
    Handle default3D = {1, 1};
    Handle default2D = {2, 1};
    Handle defaultSkybox = {3, 1};
    Handle defaultFallback = {4, 1};
    SlotMap<Shader> shaders = {};

    Shader& GetDefault3D();
    Shader& GetDefault2D();
    Shader& GetSkyboxDefault();
    Shader& GetFallback();
    Shader& GetShader(Handle h);

};



