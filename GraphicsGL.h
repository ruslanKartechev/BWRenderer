#pragma once
#include "RenderObject.h"
#include "Material.h"
#include "GameScene.h"
#include "AssetManager.h"
#include "ProjectSettings.h"

class Engine;

void GL_InitGraphics(i32 width, i32 height);

void GL_UpdateBackground(GameScene& scene);

void GL_ResizeRenderTarget(i32 width, i32 height);

void GL_RenderScene(Engine& engine);

void GL_AllocateGraphicsSkybox(RenderSubMesh& obj);

void GL_ForwardRenderOpaques(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings);

void GL_ForwardRenderTransparent(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings);

void GL_AllocateGraphicsForObject(RenderObject& obj, GameScene& scene);

void GL_InitMaterialProperties(Material& material, AssetManager& assets);

void GL_InitDefaultMaterials(AssetManager& assets);

void GL_RenderSkybox(GameScene& scene, AssetManager& assets);