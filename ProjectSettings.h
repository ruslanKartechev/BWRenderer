#pragma once
#include "string"
#include "MyTypes.h"
#include "cglm/cglm.h"

class ProjectSettings {
public:

    static std::string RootPath;
    static std::string ResourcesPath;

    vec4 Ambient_Light_Color;
    vec4 Direct_Light_Color;

    float Ambient_Light_Brightness;
    float Direct_Light_Intensity;

    float Camera_Move_Speed;
    float Camera_Rotation_Speed;

    bool Gamma_Correction;
    bool Shadows;
    bool Debug_Light_View;
    bool Debug_UVs;
    bool RenderSkyBox;

    static bool LoadProjectSettings(const std::string& filePath, ProjectSettings& outSettings);



};

