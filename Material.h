#pragma once
#include <string>
#include <vector>
#include "Handle.h"
#include "MyTypes.h"
#include "vector4.h"
#include "RenderMode.h"

using NameBindingPair = std::pair<std::string, i32>;
using NameFloatPair = std::pair<std::string, f32>;
using NameVectorPair = std::pair<std::string, vector4>;


struct MaterialTextureProp {
    Handle texHandle;
    i32 layoutIndex;

    MaterialTextureProp() {
        texHandle = {0,0};
        layoutIndex = {0};
    }

    MaterialTextureProp(Handle handle, i32 layoutIdx) {
        texHandle = Handle(handle);
        layoutIndex = layoutIdx;
    }
};



class Material {
public:
    // definition
    std::string shaderName;
    std::vector<NameFloatPair> floatsDefinitions;
    std::vector<NameVectorPair> vectorsDefinitions;
    std::vector<std::pair< std::string, NameBindingPair > > texturesDefinitions;

    // runtime
    Handle shaderHandle;
    std::vector<std::pair<u32, float>> floats;
    std::vector<std::pair<u32, vector4>> vectors;
    /// Pairs of <GL_Binding_Location ; resource Handle>
    std::vector<std::pair<u32, MaterialTextureProp>> textures;
    bool writeDepth = true;
    bool isTransparent = false;
    bool didInit = false;
    RenderMode renderMode = RenderMode::Opaque;

    void SetFloatDefinition(const std::string& name, float value);
    void SetVectorDefinition(const std::string& name, vector4 value);
    void SetTextureDefinition(const std::string& name, std::string value, i32 binding);

};