//
// Created by user on 7/3/2026.
//

#include "ShaderRegistry.h"

Shader& ShaderRegistry::GetDefault3D() {
    return shaders.GetItemRef(default3D);
}

Shader& ShaderRegistry::GetDefault2D() {
    return shaders.GetItemRef(default2D);
}

Shader& ShaderRegistry::GetSkyboxDefault() {
    return shaders.GetItemRef(defaultSkybox);
}

Shader& ShaderRegistry::GetFallback() {
    return shaders.GetItemRef(defaultFallback);
}

Shader& ShaderRegistry::GetShader(Handle h) {
    return shaders.GetItemRef(h);
}