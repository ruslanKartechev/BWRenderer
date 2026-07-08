#pragma once
#include <string>
#include <vector>
#include "Handle.h"
#include <cglm/cglm.h>

#include "MyTypes.h"

struct Material {
    // definition
    std::string shaderName;
    std::vector<std::pair<std::string, float>> floatsDefinitions;
    std::vector<std::pair<std::string, vec4>> vectorsDefinitions;
    std::vector<std::pair<std::string, std::string>> texturesDefinitions;

    // runtime
    Handle shaderHandle;
    std::vector<std::pair<u32, float>> floats;
    std::vector<std::pair<u32, vec4>> vectors;
    std::vector<std::pair<u32, std::string>> textures;
    bool writeDepth = true;
    bool isTransparent = false;
    bool didInit = false;
};