#pragma once
#include "RenderObject.h"
#include "Material.h"
#include "GameScene.h"
#include "AssetManager.h"
#include "ProjectSettings.h"
#include "Engine.h"

struct RenderTarget {
    GLuint fb;

    GLuint depthTexture;
    GLuint colorTexture;
    GLuint normalTexture;

    // GLuint rbo;
    GLuint quadVAO;
    GLuint quadVBO;
};

struct PostProcessStack {
    GLuint mainPostFBO;
    GLuint mainPostTexture;

    GLuint pingpongFBO[2];
    GLuint pingpongTextures[2];
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
void GL_ForwardRenderOpaques(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings);
void GL_ForwardRenderTransparent(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings);
void GL_RenderSkybox(GameScene& scene, AssetManager& assets);
// endregion

// region Graphics Objects Initialization
void GL_AllocateGraphicsSkybox(RenderSubMesh& obj);
void GL_AllocateGraphicsForObject(RenderObject& obj, AssetManager& assets);
// endregion
