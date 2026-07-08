#pragma once
#include "Material.h"
#include "Shader.h"
#include "SlotsMap.h"


class ShaderRegistry{
public:
    Handle default3D;
    Handle default2D;
    Handle defaultSkybox;
    Handle defaultFallback;
    Handle debugShader;

    SlotMap<Shader> shaders = {};
    SlotMap<Material> materials = {};

    Shader& GetDefault3D();
    Shader& GetDefault2D();
    Shader& GetSkyboxDefault();
    Shader& GetFallback();
    Shader& GetShader(Handle h);

};



