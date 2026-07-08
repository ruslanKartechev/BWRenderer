//
// Created by user on 7/3/2026.
//

#include "ShaderRegistry.h"

Shader& ShaderRegistry::GetDefault3D() {
    // printf("[SR]  Shader RETURNING %d, %d\n", default3D.index, default3D.generation);
    return shaders.GetItemRef(default3D);
}

Shader& ShaderRegistry::GetDefault2D() {
    // printf("[SR] Shader RETURNING %d, %d\n", default2D.index, default2D.generation);
    return shaders.GetItemRef(default2D);
}

Shader& ShaderRegistry::GetSkyboxDefault() {
    return shaders.GetItemRef(defaultSkybox);
}

Shader& ShaderRegistry::GetFallback() {
    return shaders.GetItemRef(defaultFallback);
}

Shader& ShaderRegistry::GetShader(Handle h) {
    // printf("[SR] Getting item ref %d, %d \n", h.index, h.generation);
    return shaders.GetItemRef(h);
}