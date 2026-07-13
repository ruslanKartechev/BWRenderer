#pragma once
#include <string>
#include <vector>
#include "Handle.h"
#include "MyTypes.h"

struct vector4 {
    float x;
    float y;
    float z;
    float w;
};

enum class RenderMode {
    Opaque = 0,
    Transparent = 1,
};

class Material {
public:
    // definition
    std::string shaderName;
    std::vector<std::pair<std::string, float>> floatsDefinitions;
    std::vector<std::pair<std::string, vector4>> vectorsDefinitions;
    std::vector<std::pair<std::string, std::string>> texturesDefinitions;

    // runtime
    Handle shaderHandle;
    std::vector<std::pair<u32, float>> floats;
    std::vector<std::pair<u32, vector4>> vectors;
    /// Pairs of <GL hUniform Location, handle to the texture rresource
    std::vector<std::pair<u32, Handle>> textures;
    bool writeDepth = true;
    bool isTransparent = false;
    bool didInit = false;
    RenderMode renderMode = RenderMode::Opaque;

    void SetFloatDefinition(const std::string& name, float value);
    void SetVectorDefinition(const std::string& name, vector4 value);
    void SetTextureDefinition(const std::string& name, std::string value);

};