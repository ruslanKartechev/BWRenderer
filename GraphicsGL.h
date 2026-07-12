#pragma once
#include "RenderObject.h"
#include "Material.h"
#include "GameScene.h"
#include "AssetManager.h"


void GL_UpdateBackground(GameScene scene);

void GL_AllocateGraphicsSkybox(RenderSubMesh& obj);

void GL_ForwardRenderOpaques(GameScene& scene, Camera& camera, AssetManager& assets);

void GL_AllocateGraphicsForObject(RenderObject& obj, GameScene& scene);

void GL_InitMaterialProperties(Material& material, AssetManager& assets);

void GL_InitDefaultMaterials(AssetManager& assets);

void GL_RenderSkybox(GameScene& scene, AssetManager& assets);