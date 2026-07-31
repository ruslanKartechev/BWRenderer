#pragma once
#include "string"
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

    float BloomExposure = 1.2f;
    float BloomThreshold = 0.5f;
    float GammaValue = 2.2;

    bool UseGammaCorrection;
    bool RenderShadows;
    bool RenderSkyBox;
    bool PostProcess;

    bool PostProcess_SSR;
    bool PostProcess_Bloom;
    bool PostProcess_DOF;
    bool PostProcess_SSAO;
    bool PostProcess_ToneMapping;
    bool PostProcess_BW;

    bool DebugVisualizeLightSources;

    bool DevRenderUvs;
    bool DevRenderDepths;
    bool DevRenderNormals;
    bool DevRenderColors;

    static bool LoadProjectSettings(const std::string& filePath, ProjectSettings& outSettings);

};

