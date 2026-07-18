#pragma once
#include "RenderObject.h"
#include "Material.h"
#include "GameScene.h"
#include "AssetManager.h"
#include "ProjectSettings.h"
#include "Engine.h"
#include "InstancedBuffer.h"

struct RenderTarget {
    i32 width;
    i32 height;

    /// G-Buffer for deferred pipeline. Main render buffer for forward pipeline
    GLuint mainFB;
    GLuint depthTexture;
    GLuint colorTexture;
    GLuint normalTexture;

    // GLuint rbo;
    GLuint quadVAO;
    GLuint quadVBO;
};

struct BloomMipmap {
    GLuint fbo {0};
    GLuint texture {0};
    float sizeX {1};
    float sizeY {1};

};

struct PostProcessStack {
    GLuint postProcessA_FB {0};
    GLuint postProcessA_Texture {0};
    GLuint postProcessA_DepthTex {0};

    GLuint postProcessB_FB {0};
    GLuint postProcessB_Texture {0};
    GLuint postProcessB_DepthTex {0};

    std::vector<BloomMipmap> bloomMipmaps {};
};


// region Materials
void GL_InitMaterialProperties(Material& material, AssetManager& assets);
void GL_InitDefaultMaterials(AssetManager& assets);
void GL_InitMaterialParametersDefault3D(Material& material);
// endregion

// region Graphics
void GL_InitGraphics(i32 width, i32 height);
void GL_ResizeRenderTarget(i32 width, i32 height);
// endregion

// region Rendering
void GL_RenderScene(Engine& engine);
void GL_OpaquePass(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings);
void GL_ForwardRenderTransparent(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings);
void GL_SkyboxPass(GameScene& scene, AssetManager& assets, u32 frameBuffer);

void GL_AddInstanceBuffer(std::shared_ptr<InstancedBuffer> bufferPtr);
// endregion

// region Graphics Objects Initialization
void GL_AllocateGraphicsSkybox(RenderSubMesh& obj);
void GL_AllocateGraphicsForObject(RenderObject& obj, AssetManager& assets);
void GL_AllocateGraphicsForMesh(RenderSubMesh& meshRenderData, AssetManager& assets);
// endregion
