#include <iostream>
#include "GraphicsGL.h"
#include "Material.h"
#include "Uniforms.h"
#include "Camera.h"
#include "Engine.h"
#include "Terrain.h"
#include "cglm/cglm.h"
#include "Mesh.h"

#define DRAW_DEBUG_TERRAIN_COLORS


static RenderTarget renderTarget = {};
static PostProcessStack ppStack = {};
static std::vector<std::shared_ptr<InstancedBuffer> > InstancedBuffers = {};


static constexpr u32 RENDER_TEX_TYPE = GL_FLOAT;
static constexpr i32 RENDER_TEX_STORE_FORMAT = GL_RGBA16F;
static constexpr u32 RENDER_TEX_IN_FORMAT = GL_RGBA;


// region Helpers
static void GL_CleanState() {
    glBindVertexArray(0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glBindTexture(GL_TEXTURE_2D, 0);
}

static void GL_PrepareRenderScreenQuad() {
    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);
    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);
}

static void GL_PrepareRenderFrameBufferToScreen(GLuint frameBuffer = 0) {
    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer); // default framebuffer
    GL_PrepareRenderScreenQuad();
}

static void GL_RenderScreenQuad(const RenderTarget& target) {
    glBindVertexArray(target.quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    GL_CleanState();
}

static void GL_RenderTargetToScreen(const RenderTarget& target,
                                    u32 shaderID) {
    GL_PrepareRenderFrameBufferToScreen(0);
    glUseProgram(shaderID);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    GL_RenderScreenQuad(target);
}

void GL_RenderPostProcessBufferToScreen(const PostProcessStack& stack,
                                        const ProjectSettings& settings,
                                        AssetManager& assets,
                                        bool aBuffer) {

    Shader& screenShader = assets.GetShader(assets.shaderScreenRenderTexture);
    glUseProgram(screenShader.GetShaderId());

    f32 gammaValue = settings.UseGammaCorrection ? settings.GammaValue : 1.0;
    screenShader.SetFloat("_GAMMA", gammaValue);
    // std::cout << "Gamme value " << gammaValue << "  settings.UseGammaCorrection" << settings.UseGammaCorrection << std::endl;

    GL_PrepareRenderFrameBufferToScreen(0);
    auto texture = aBuffer ? stack.postProcessA_Texture : stack.postProcessB_Texture;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, texture);
    GL_RenderScreenQuad(renderTarget);
}

//endregion


// region Materials
void GL_InitMaterialParametersDefault3D(Material& material) {
    material.shaderName = Shader_DefaultDeferredG;
    const f32 shade = 0.96f;
    material.SetVectorDefinition(ID_COLOR_TINT, {shade, shade, shade, 1.0});

    material.SetFloatDefinition(ID_SMOOTHNESS, .25f);
    material.SetFloatDefinition(ID_METALLIC, .1f);
    material.SetFloatDefinition(ID_SSR_POWER, 0.0f);

    material.SetTextureDefinition(ID_BASE_MAP, "default_white", 0);
    material.SetTextureDefinition(ID_NORMAL_MAP, "default_normal", 1);
    material.SetTextureDefinition(ID_SKYBOX_CUBEMAP, "Skybox", 2);

    material.SetVectorDefinition(ID_BASE_MAP_TO, {1.0f, 1.0f, 0.0f, 0.0f});
}


void GL_InitDefaultMaterials(AssetManager& assets) {
    // Default 3D
    {
        auto& material = assets.materials.GetNewObjectAndHandle(assets.materialDefault3d);
        GL_InitMaterialParametersDefault3D(material);
        GL_InitMaterialProperties(material, assets);
    }
    // Default 2d
    {
        auto& material = assets.materials.GetNewObjectAndHandle(assets.materialDefault2d);
        material.shaderName = "Default2D";
        material.SetVectorDefinition(ID_COLOR_TINT, {0.9f, 0.9f, 0.9f, 1.0});
        GL_InitMaterialProperties(material, assets);
    }
    // Debug Shader
    {
        auto& material = assets.materials.GetNewObjectAndHandle(assets.materialDebug);
        material.shaderName = "DebugShader";
        GL_InitMaterialProperties(material, assets);
    }
    // Skybox
    {
        auto& material = assets.materials.GetNewObjectAndHandle(assets.materialSkybox);
        material.shaderName = Shader_SkyboxDefault;
        material.SetTextureDefinition(ID_SKYBOX_CUBEMAP, "Skybox", 0);
        material.SetFloatDefinition(ID_SKYBOX_BRIGHTNESS, 0.5f);
    }
}


void GL_InitMaterialProperties(Material& material,
                               AssetManager& assets) {
    material.didInit = true;
    std::cout << "\n[Graphics][MaterialProperties] Shader: " << material.shaderName << std::endl;
    material.shaderHandle = assets.FindShaderByName(material.shaderName.c_str());

    if (material.shaderHandle.IsEmpty()) {
        std::cerr << "[Graphics] Failed to find shader: " << material.shaderName << std::endl;
        return;
    }
    auto& shader = assets.shaders.GetItemRef(material.shaderHandle);
    auto shaderId = shader.GetShaderId();
    glUseProgram(shaderId);

    for (auto i = 0; i < material.floatsDefinitions.size(); i++) {
        auto& paramName = material.floatsDefinitions[i].first;
        u32 uniformLocation = glGetUniformLocation(shaderId, paramName.c_str());
        std::cout<<"[Material.Float] [shader "<<shaderId<<"] Name: "<<paramName<<", uniformLocation: "<<uniformLocation<<std::endl;
        material.floats.emplace_back(uniformLocation, material.floatsDefinitions[i].second);
    }

    for (auto i = 0; i < material.vectorsDefinitions.size(); i++) {
        auto& paramName = material.vectorsDefinitions[i].first;
        i32 uniformLocation = glGetUniformLocation(shaderId, paramName.c_str());
        std::cout << "[Material.Vector] [shader " << shaderId << "] Name: " << paramName << ", uniformLocation: " << uniformLocation << std::endl;
        material.vectors.emplace_back(uniformLocation, material.vectorsDefinitions[i].second);
    }

    for (auto i = 0; i < material.texturesDefinitions.size(); i++) {
        assets.GetDefault3D();
        auto& paramName = material.texturesDefinitions[i].first;
        auto& nameBindingPair = material.texturesDefinitions[i].second;
        auto& assetName = nameBindingPair.first;
        i32 uniformLocation = nameBindingPair.second;

        Handle textureHandle = {};
        if (assetName.empty()) {
            textureHandle.Copy(assets.defaultWhiteTexture);
        }
        else {
            textureHandle = assets.FindTextureByName(assetName.c_str());
        }

        if (textureHandle.IsEmpty()) {
            textureHandle.Copy(assets.defaultWhiteTexture);
            if (!assetName.empty()) {
                std::cerr << "[Material.Texture] Failed to find texture: '" << assetName << "'" << std::endl;
            }
        }
        else {
            std::cout<<"[Material.Texture] Found Texture: " << assetName<<std::endl;
        }

        material.textures.emplace_back(uniformLocation, MaterialTextureProp(textureHandle, uniformLocation));
        std::cout << "[Material.Texture] [shader " << shaderId << "] Param: " << paramName << " Asset: " << assetName << " uniformLocation: " << uniformLocation << std::endl;
    }
    glUseProgram(0);

    std::cout << std::endl << std::endl;
}

// endregion


// region Render-Target Framebuffer INIT
void GL_ResizeRenderTarget(i32 width,
                           i32 height) {
    RenderTarget& target = renderTarget;
    PostProcessStack& stack = ppStack;

    target.width = width;
    target.height = height;

    glViewport(0, 0, width, height);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, target.normalTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);
    glBindTexture(GL_TEXTURE_2D, target.depthTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);


    glBindTexture(GL_TEXTURE_2D, stack.postProcessA_DepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, stack.postProcessB_DepthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, stack.postProcessA_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);

    glBindTexture(GL_TEXTURE_2D, stack.postProcessB_Texture);
    glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);

    i32 lvl = 0;
    for (auto& mm: stack.bloomMipmaps) {
        f32 divisor = (f32) pow(2.0, lvl + 1);
        f32 mip_width = ((f32) width / divisor);
        f32 mip_height = ((f32) height / divisor);
        glBindTexture(GL_TEXTURE_2D, mm.texture);
        glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, mip_width, mip_height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);
        lvl++;
    }

    // glBindRenderbuffer(GL_RENDERBUFFER, stack.postProcessA_DepthRBO);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, stack.postProcessA_DepthRBO);
    //
    // glBindRenderbuffer(GL_RENDERBUFFER, stack.postProcessB_DepthRBO);
    // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
    // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, stack.postProcessB_DepthRBO);


    GL_CleanState();
}


void GL_BuildPostProcessStack(PostProcessStack& stack,
                              i32 width,
                              i32 height) {
    // Buffer A
    {
        glGenFramebuffers(1, &stack.postProcessA_FB);
        glGenTextures(1, &stack.postProcessA_Texture);

        glBindFramebuffer(GL_FRAMEBUFFER, stack.postProcessA_FB);
        glBindTexture(GL_TEXTURE_2D, stack.postProcessA_Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, stack.postProcessA_Texture, 0);

        glGenTextures(1, &stack.postProcessA_DepthTex);
        glBindTexture(GL_TEXTURE_2D, stack.postProcessA_DepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, stack.postProcessA_DepthTex, 0);

        // glGenRenderbuffers(1, &stack.postProcessA_DepthRBO);
        // glBindRenderbuffer(GL_RENDERBUFFER, stack.postProcessA_DepthRBO);
        // glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT24, width, height);
        // glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, stack.postProcessA_DepthRBO);
    }
    // Buffer B
    {
        glGenFramebuffers(1, &stack.postProcessB_FB);
        glGenTextures(1, &stack.postProcessB_Texture);

        glBindFramebuffer(GL_FRAMEBUFFER, stack.postProcessB_FB);
        glBindTexture(GL_TEXTURE_2D, stack.postProcessB_Texture);
        glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, stack.postProcessB_Texture, 0);

        glGenTextures(1, &stack.postProcessB_DepthTex);
        glBindTexture(GL_TEXTURE_2D, stack.postProcessB_DepthTex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, stack.postProcessB_DepthTex, 0);
    }


    // Bloom

    // Bloom effect FBOs
    constexpr i32 MIP_LEVELS = 5;
    stack.bloomMipmaps.reserve(MIP_LEVELS + 1);

    for (i32 i = 0; i < MIP_LEVELS; i++) {
        BloomMipmap& mip = stack.bloomMipmaps.emplace_back();
        f32 divisor = (f32) pow(2.0, i + 1);
        f32 mip_width = ((f32) width / divisor);
        f32 mip_height = ((f32) height / divisor);
        mip.sizeX = mip_width;
        mip.sizeY = mip_height;
        // printf("--->> Created MIP level (%d) Dimensions: (%d, %d)\n\n", (i+1), (i32)mip_width, (i32)mip_height);

        glGenFramebuffers(1, &mip.fbo);
        glGenTextures(1, &mip.texture);

        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);
        glBindTexture(GL_TEXTURE_2D, mip.texture);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA16F, (i32) mip_width, (i32) mip_height, 0, GL_RGBA, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, mip.texture, 0);
    }

    GL_CleanState();
}


void GL_BuildRenderTarget(RenderTarget& target,
                          i32 width,
                          i32 height) {
    f32 quadVertices[] = {
        // positions    // uvs
        -1.0f, +1.0f,   0.0f, 1.0f,
        -1.0f, -1.0f,   0.0f, 0.0f,
        +1.0f, -1.0f,   1.0f, 0.0f,
        -1.0f, +1.0f,   0.0f, 1.0f,
        +1.0f, -1.0f,   1.0f, 0.0f,
        +1.0f, +1.0f,   1.0f, 1.0f
    };

    target.width = width;
    target.height = height;

    glGenVertexArrays(1, &target.quadVAO);
    glGenBuffers(1, &target.quadVBO);
    glBindVertexArray(target.quadVAO);
    glBindBuffer(GL_ARRAY_BUFFER, target.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

    // Vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (const void*) (0 * sizeof(f32)));
    // UVs
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(f32), (const void*) (2 * sizeof(f32)));
    // Create framebuffer
    glGenFramebuffers(1, &target.mainFB);
    glBindFramebuffer(GL_FRAMEBUFFER, target.mainFB);

    {
        // Color texture (Attachment 0)
        glGenTextures(1, &target.colorTexture);
        glBindTexture(GL_TEXTURE_2D, target.colorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, RENDER_TEX_TYPE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Bind to FB
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.colorTexture, 0);
    }
    {
        // Depth Texture (Depth Attachment)
        glGenTextures(1, &target.depthTexture);
        glBindTexture(GL_TEXTURE_2D, target.depthTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, width, height, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, target.depthTexture, 0);
    }
    {
        // Normals texture (Attachment 1)
        glGenTextures(1, &target.normalTexture);
        glBindTexture(GL_TEXTURE_2D, target.normalTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, RENDER_TEX_STORE_FORMAT, width, height, 0, RENDER_TEX_IN_FORMAT, RENDER_TEX_TYPE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        // Bind to FB
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, target.normalTexture, 0);
    }
    // Attach both textures to attachments
    GLuint attachments[2] = {GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
    glDrawBuffers(2, attachments);

    // unbind into default state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GL_InitGraphics(i32 width,
                     i32 height) {
    std::cout << "[Graphics] Init " << width << "height: " << height << std::endl;
    GL_BuildRenderTarget(renderTarget, width, height);
    GL_BuildPostProcessStack(ppStack, width, height);
    glDisable(GL_FRAMEBUFFER_SRGB);
}
// endregion


// region Graphics Allocation
void GL_AllocateGraphicsSkybox(RenderSubMesh& obj) {
    f32 skyboxVertices[] = {
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, -1.0f, 1.0f,
        -1.0f, -1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        1.0f,  1.0f, -1.0f,
        1.0f, 1.0f, 1.0f,
        1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, 1.0f,
        -1.0f, 1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, -1.0f,
        1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f, 1.0f,
        1.0f, -1.0f, 1.0f
    };
    glGenVertexArrays(1, &obj.vao);
    glGenBuffers(1, &obj.vbo);
    glBindVertexArray(obj.vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(f32), (void*) 0);
    glBindVertexArray(0);
}


void GL_AllocateGraphicsMesh(RenderSubMesh& meshRenderData,
                                AssetManager& assets) {
    Mesh& mesh = assets.meshes.GetItemRef(meshRenderData.hMesh);

    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * mesh.vertexDataCount, mesh.vertexData, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i32) * mesh.indexCount, mesh.indexData, GL_DYNAMIC_DRAW);

    i32 strideBytes = mesh.stride * sizeof(f32);
    i32 attributeIdx = 0;
    glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, strideBytes, (const void*)(mesh.startIndexVertex * sizeof(f32) )); // XYZ
    glEnableVertexAttribArray(attributeIdx);
    attributeIdx++;
    if (mesh.startIndexUV >= 0) {
        glVertexAttribPointer(attributeIdx, 2, GL_FLOAT, GL_FALSE, strideBytes, (const void*) (mesh.startIndexUV * sizeof(f32) )); // UV
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }
    if (mesh.startIndexNormals >= 0) {
        glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, strideBytes, (const void*) (mesh.startIndexNormals * sizeof(f32))); // XYZ normals
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }
    if (mesh.startIndexColor >= 0) {
        glVertexAttribPointer(attributeIdx, 4, GL_FLOAT, GL_FALSE, strideBytes, (const void*) (mesh.startIndexColor * sizeof(f32))); // RGBA
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }
    if (mesh.startIndexTangent >= 0) {
        // Tangents
        glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, strideBytes, (const void*) (mesh.startIndexColor * sizeof(f32))); // RGBA
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
    }

    glBindVertexArray(0);
    meshRenderData.vao = vao;
    meshRenderData.vbo = vbo;
    meshRenderData.ebo = ebo;
}



void GL_AllocateGUIQuad(UIObject& obj) {
    i32 stride = 4;
    i32 startIndexVertex = 0;
    i32 startIndexUV = 2;

    i32 vertexDataCount = 4 * stride;
    i32 indexCount = 6;
    f32* vertexData = new f32[] {
        // POSITION    // UV
        0.0f,  0.0f,   0.0f, 0.0f, // 0
        1.0f,  0.0f,   1.0f, 0.0f, // 1
        1.0f,  1.0f,   1.0f, 1.0f, // 2
        0.0f,  1.0f,   0.0f, 1.0f, // 3
    };
    i32* indexData = new i32[] {
        0, 1, 2, 0, 2, 3
    };

    GLuint vao;
    GLuint vbo;
    GLuint ebo;
    i32 strideBytes = stride * sizeof(f32);

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    // Positions
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * vertexDataCount, vertexData, GL_DYNAMIC_DRAW);
    // Triangles
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i32) * indexCount, indexData, GL_DYNAMIC_DRAW);

    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, strideBytes, (const void*)(startIndexVertex * sizeof(f32) )); // XYZ
    glEnableVertexAttribArray(0);

    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, strideBytes, (const void*) (startIndexUV * sizeof(f32) )); // UV
    glEnableVertexAttribArray(1);

    obj.vao = vao;
    obj.vbo = vbo;
    obj.ebo = ebo;
}


void GL_AllocateClipmapMesh(ClipmapMesh& mesh, bool addUvs = false) {
    GLuint vao;
    GLuint vbo;
    GLuint ebo;

    glGenVertexArrays(1, &vao);
    glGenBuffers(1, &vbo);
    glGenBuffers(1, &ebo);
    glBindVertexArray(vao);
    glBindBuffer(GL_ARRAY_BUFFER, vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(f32) * mesh.vertexDataCount, mesh.vertexDataPtr, GL_DYNAMIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(i32) * mesh.indexCount, mesh.indexDataPtr, GL_DYNAMIC_DRAW);

    i32 stride = mesh.stride * sizeof(f32);

    // Vertex Positions
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(0 * sizeof(f32) )); // XYZ
    glEnableVertexAttribArray(0);
    if (addUvs) {
        // Vertex UVs
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(3 * sizeof(f32))); // UV
        glEnableVertexAttribArray(1);
    }

    glBindVertexArray(0);
    mesh.vao = vao;
    mesh.vbo = vbo;
    mesh.ebo = ebo;

}

void GL_AllocateGraphicsTerrain(Terrain& terrain) {
    GL_AllocateClipmapMesh(terrain.centerMesh);
    GL_AllocateClipmapMesh(terrain.blockMesh);
    GL_AllocateClipmapMesh(terrain.fixUpXMesh);
    GL_AllocateClipmapMesh(terrain.fixUpZMesh);
    GL_AllocateClipmapMesh(terrain.trimXMesh);
    GL_AllocateClipmapMesh(terrain.trimZMesh);

    GL_AllocateClipmapMesh(terrain.zeroAreaX1);
    GL_AllocateClipmapMesh(terrain.zeroAreaX2);
    GL_AllocateClipmapMesh(terrain.zeroAreaZ1);
    GL_AllocateClipmapMesh(terrain.zeroAreaZ2);
}


void GL_AllocateGraphicsForObject(RenderObject& obj,
                                  AssetManager& assets) {
    obj.subMeshses.reserve(1);
    if (obj.subMeshses.size() < 1) {
        obj.subMeshses.resize(1);
    }
    for (auto& meshRenderData: obj.subMeshses) {
        GL_AllocateGraphicsMesh(meshRenderData, assets);
    }
}
// endregion



// region Rendering Loop
void BindRenderTarget(const RenderTarget& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target.mainFB);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}


void GL_BindMaterialTextures(Material& material, AssetManager& assets, bool log = false) {
    for (auto& texturePair: material.textures) {
        i32 binding = texturePair.first;
        MaterialTextureProp& texProp = texturePair.second;
        Texture& texture = assets.textures.GetItemRef(texProp.texHandle);
        if (assets.textures.IsNullItem(texture)) {
            // std::cerr<<"Failed to load texture"<<std::endl;
            continue;
        }

        auto type = GL_TEXTURE_2D;
        switch (texture.pixelFormat) {
            case Texture::TEX_FORMAT_sRGB32:
                type = GL_TEXTURE_2D;
                break;
            case Texture::TEX_FORMAT_R32:
                type = GL_TEXTURE_2D;
                break;
            case Texture::TEX_FORMAT_CUBEMAP:
                type = GL_TEXTURE_CUBE_MAP;
                break;
            default:
                type = GL_TEXTURE_2D;
                break;
        }
        auto texBindingLocation = GL_TEXTURE0 + binding;
        glActiveTexture(texBindingLocation);
        glBindTexture(type, texture.glHandle);
        // if (log)
        // {
        //     printf("[R] [%s] texBindingLocation: %d, UniformBinding: %d, TextureHandleIndex %d, glHandle %d , size (%d,%d)\n",
        //         texture.name, texBindingLocation, binding, texProp.texHandle.index, texture.glHandle, texture.width, texture.height);
        // }
    }
}

void GL_UseMaterial(Material& material,
                    u32 shaderId,
                    AssetManager& assets) {

    glUseProgram(shaderId);
    for (auto& floatPair: material.floats) {
        glUniform1f(floatPair.first, floatPair.second);
    }
    for (auto& vecPair: material.vectors) {
        vec4 vec = {vecPair.second.x, vecPair.second.y, vecPair.second.z, vecPair.second.w};
        glUniform4fv(vecPair.first, 1, (const f32*) vec);
    }
    GL_BindMaterialTextures(material, assets);
}


void GL_OpaquePass(GameScene& scene,
                   Camera& camera,
                   AssetManager& assets,
                   ProjectSettings& settings) {
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.mainFB); // Main frame buffer

    scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    Transform& cameraTransform = scene.transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);

    vec4& backgroundColor = scene.backgroundColor;
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);

    for (auto& objHandle: scene.activeWorldHandles) {
        if (scene.worldObjectsPool.IsValid(objHandle) == false) {
            continue;
        }
        RenderObject& obj = scene.worldObjectsPool.GetItemRef(objHandle);
        Transform& transform = scene.transforms.GetItemRef(obj.hTransform);

        for (auto& renderData: obj.subMeshses) {

            Material& material = assets.materials.GetItemRef(renderData.hMaterial);
            Shader& shader = assets.GetShader(material.shaderHandle);
            Mesh& mesh = assets.meshes.GetItemRef(renderData.hMesh);
            bool isNull = assets.shaders.IsNullItem(shader);
            if (isNull) {
                continue;
            }
            auto shaderId = shader.GetShaderId();
            GL_UseMaterial(material, shaderId, assets);

            i32 model_Location = glGetUniformLocation(shaderId, ID_UNIFORM_MODEL);
            i32 view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
            i32 proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);
            glUniformMatrix4fv(model_Location, 1, GL_FALSE, (f32*) transform.modelMatrix);
            glUniformMatrix4fv(view_Location, 1, GL_FALSE, (f32*) camera.viewMatrix);
            glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (f32*) camera.projectionMatrix);

            glBindVertexArray(renderData.vao);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
            // printf("opaque object vao %d \n", renderData.vao);

            // Reset State
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}


void ClearBackgroundNoSkyBox() {}


void GL_SkyboxPass(GameScene& scene,
                   AssetManager& assets,
                   u32 frameBuffer) {
    auto& skybox = scene.skybox;
    auto& skyboxMaterial = assets.materials.GetItemRef(skybox.renderData.hMaterial);
    auto& camera = scene.camera;
    auto& shader = assets.shaders.GetItemRef(skyboxMaterial.shaderHandle);
    auto shaderId = shader.GetShaderId();

    mat4 viewNoTranslation;
    camera.GetViewMatrixNoTranslation(viewNoTranslation);

    glBindFramebuffer(GL_FRAMEBUFFER, frameBuffer);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);

    glDepthFunc(GL_LEQUAL);
    glUseProgram(shaderId);

    if (!skyboxMaterial.textures.empty()) {
        auto& texturePair = skyboxMaterial.textures[0];
        auto& textureProp = texturePair.second;
        auto& texture = assets.textures.GetItemRef(textureProp.texHandle);
        if (!assets.textures.IsNullItem(texture)) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture.glHandle);
        }
    }

    glUniformMatrix4fv(glGetUniformLocation(shaderId, ID_UNIFORM_VIEW), 1, GL_FALSE, (f32*) viewNoTranslation);
    glUniformMatrix4fv(glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION), 1, GL_FALSE, (f32*) camera.projectionMatrix);

    for (auto& floatPair: skyboxMaterial.floats) {
        glUniform1f(floatPair.first, floatPair.second);
    }
    for (auto& vecPair: skyboxMaterial.vectors) {
        vec4 vec = {vecPair.second.x, vecPair.second.y, vecPair.second.z, vecPair.second.w};
        glUniform4fv(vecPair.first, 1, (const f32*) vec);
    }

    glBindVertexArray(skybox.renderData.vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glDepthFunc(GL_LESS);

    GL_CleanState();
}



void GL_BlitDepthFromGBuffer(RenderTarget target,
                             PostProcessStack stack) {

    glBindFramebuffer(GL_READ_FRAMEBUFFER, target.mainFB);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, stack.postProcessA_FB);
    glBlitFramebuffer(0, 0, target.width, target.height, 0, 0, target.width, target.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);

    // glBindFramebuffer(GL_READ_FRAMEBUFFER, target.mainFB);
    // glBindFramebuffer(GL_DRAW_FRAMEBUFFER, stack.postProcessB_FB);
    // glBlitFramebuffer(0, 0, target.width, target.height, 0, 0, target.width, target.height, GL_DEPTH_BUFFER_BIT, GL_NEAREST);
}


void GL_DeferredLightingPass(RenderTarget target,
                             PostProcessStack stack,
                             GameScene& scene,
                             Camera& camera,
                             AssetManager& assets) {

    auto outputFB = stack.postProcessA_FB;
    GL_PrepareRenderFrameBufferToScreen(outputFB);
    glDisable(GL_DEPTH_TEST);

    Transform& cameraTransform = scene.transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    mat4 inverseProjection;
    mat4 inverseView;

    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);
    glm_mat4_inv(camera.projectionMatrix, inverseProjection);
    glm_mat4_inv(camera.viewMatrix, inverseView);

    auto& shader = assets.GetShader(assets.shaderDefault3DLight);
    auto shaderId = shader.GetShaderId();
    glUseProgram(shaderId);
    // std::cout << "SHADER FOR LIGHTING: " << shaderId << " " << shader.GetName() << std::endl;

    shader.SetVec3(ID_UNIFORM_VIEW_POS, cameraTransform.position);
    shader.SetVec3(ID_UNIFORM_AMBIENT_LIGHT_COLOR, scene.ambientLightColor);
    shader.SetFloat(ID_UNIFORM_AMBIENT_LIGHT_INTENSITY, scene.ambientIntensity);
    shader.SetMat4("MATRIX_INVERSE_PROJECTION", inverseProjection);
    shader.SetMat4("MATRIX_INVERSE_VIEW", inverseView);

    shader.SetVec3("DIRECTIONAL_LIGHT.direction", mainLightDir);
    shader.SetVec3("DIRECTIONAL_LIGHT.color", scene.mainLight.color);
    shader.SetFloat("DIRECTIONAL_LIGHT.intensity", scene.mainLight.intensity);

    // Binding g buffers
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, target.normalTexture);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, target.depthTexture);

    auto skyMatHandle = scene.skybox.renderData.hMaterial;
    auto& skyboxMat = assets.materials.GetItemRef(skyMatHandle);
    if (!skyboxMat.textures.empty()) {
        auto& texturePair = skyboxMat.textures[0];
        auto& textureProp = texturePair.second;
        auto& texture = assets.textures.GetItemRef(textureProp.texHandle);

        if (!assets.textures.IsNullItem(texture)) {

            glActiveTexture(GL_TEXTURE3);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texture.glHandle);
        }
    }

    GL_RenderScreenQuad(target);
    glEnable(GL_DEPTH_TEST);
}

// endregion



// region Dev-Render options
void GL_RenderDepthOnly(const RenderTarget& target,
                        Shader& shader,
                        Camera& camera) {
    GL_PrepareRenderFrameBufferToScreen(0);
    auto shaderId = shader.GetShaderId();
    glUseProgram(shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target.depthTexture);
    glUniform1i(glGetUniformLocation(shaderId, ID_DEPTH_TEX), 0);

    glUniform1f(glGetUniformLocation(shaderId, ID_FAR_PLANE), camera.farPlane);
    glUniform1f(glGetUniformLocation(shaderId, ID_NEAR_PLANE), camera.nearPlane);

    GL_RenderScreenQuad(target);
}

void GL_RenderNormalsOnly(const RenderTarget& target,
                          Shader& shader,
                          Camera& camera) {
    GL_PrepareRenderFrameBufferToScreen(0);
    auto shaderId = shader.GetShaderId();
    glUseProgram(shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target.normalTexture);
    glUniform1i(glGetUniformLocation(shaderId, ID_NORMALS_TEX), 0);

    GL_RenderScreenQuad(target);
}

void GL_RenderColorsOnly(const RenderTarget& target,
                         Shader& shader,
                         Camera& camera) {
    GL_PrepareRenderFrameBufferToScreen(0);
    auto shaderId = shader.GetShaderId();
    glUseProgram(shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glUniform1i(glGetUniformLocation(shaderId, ID_COLOR_TEX), 0);

    GL_RenderScreenQuad(target);
}

// endregion


// region Instancing
void GL_AddInstanceBuffer(std::shared_ptr<InstancedBuffer> bufferPtr) {
    InstancedBuffers.push_back(bufferPtr);
    auto& buffer = *bufferPtr;
    glGenBuffers(1, &buffer.arrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, buffer.arrayObject);
    glBufferData(GL_ARRAY_BUFFER, buffer.entries.size() * sizeof(mat4), buffer.entries.data(), GL_STATIC_DRAW);

    glBindVertexArray(buffer.renderData.vao); // VAO is MESH data
    constexpr size_t colSize = sizeof(vec4);
    constexpr i32 uniformIdxStart = 3;

    // Column-per-column uniforms declaration
    for (i32 i = 0; i < 4; i++) {
        i32 uni = uniformIdxStart + i;
        glEnableVertexAttribArray(uni);
        // (2) THIS size is in 'floats' (not bytes)     // (5) Here sizeof(mat4) is 4*4*4 = 64 bytes
        glVertexAttribPointer(uni, 4, GL_FLOAT, GL_FALSE, sizeof(mat4), (void*) (i * colSize));
        glVertexAttribDivisor(uni, 1);
    }

    glBindVertexArray(0);
}

void GL_InstancedPass(RenderTarget& target, AssetManager& assets,
                      Camera& camera) {

    glBindFramebuffer(GL_FRAMEBUFFER, target.mainFB);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);

    // std::cout << "Rendering "<< InstancedBuffers.size() << " Instanced buffers" << std::endl;
    for (auto& ptr: InstancedBuffers) {
        auto& buffer = *ptr;
        auto& mesh = assets.meshes.GetItemRef(buffer.renderData.hMesh);
        auto& material = assets.materials.GetItemRef(buffer.renderData.hMaterial);
        auto& shader = assets.shaders.GetItemRef(material.shaderHandle);
        auto shaderId = shader.GetShaderId();
        GL_UseMaterial(material, shaderId, assets);

        if (buffer.isDirty) {
            buffer.isDirty = false;
            GLsizeiptr dataSize = sizeof(mat4) * buffer.entries.size();
            glBindBuffer(GL_ARRAY_BUFFER, buffer.arrayObject);
            glBufferSubData(GL_ARRAY_BUFFER, 0, dataSize, buffer.entries.data());
            glBindBuffer(GL_ARRAY_BUFFER, 0);
        }
        glBindVertexArray(buffer.renderData.vao);
        // printf("Rendering shader id (%d)  view_Location (%d), proj_Location (%d)\n", shaderId, view_Location, proj_Location);

        i32 view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
        i32 proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);
        glUniformMatrix4fv(view_Location, 1, GL_FALSE, (f32*) camera.viewMatrix);
        glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (f32*) camera.projectionMatrix);

        glDrawElementsInstanced(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, 0, buffer.entries.size());
        glBindVertexArray(0);
    }
}
// endregion

//region Terrain render
// TODO FIX THE degenerate triangle strips at the bottom and left (rotation is flipped)
void GL_TerrainPass(RenderTarget& renderTarget, GameScene& scene, Camera& camera, AssetManager& assets) {

    if (scene.terrain.isBuilt == false) {
        std::cerr << "no terrain built" << std::endl;
        return;
    }
    Terrain& terrain = scene.terrain;
    Material& material = assets.materials.GetItemRef(terrain.hMaterial);

    if (assets.materials.IsNullItem(material)) {
        std::cerr << "No terrain material" << std::endl;
        return;
    }

    Transform& transform = scene.transforms.GetItemRef(terrain.hTransform);
    Shader& shader = assets.shaders.GetItemRef(material.shaderHandle);
    auto shaderId = shader.GetShaderId();

    f32 worldSize = terrain.worldSize;
    vec4 sizeScaleVec = {
        (worldSize),
        (worldSize),
        terrain.heightData.scale,
        1.0
    };
    // Get frustum planes
    mat4 viewProj;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProj);
    vec4 frustumPlanes[6];
    glm_frustum_planes(viewProj, frustumPlanes); // left, right, bottom, top, near, far

    // 2 AABB frustum culling
    auto IsInsideFrustum = [](vec3 min, vec3 max, vec4 planes[6]) -> bool {
        for (int i = 0; i < 6; i++) {
            f32 px = planes[i][0] > 0.0f ? max[0] : min[0];
            f32 py = planes[i][1] > 0.0f ? max[1] : min[1];
            f32 pz = planes[i][2] > 0.0f ? max[2] : min[2];
            f32 distance = (px * planes[i][0]) + (py * planes[i][1]) + (pz * planes[i][2]) + planes[i][3];
            if (distance < 0.0f) {
                return false;
            }
        }
        return true;
    };

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_BLEND);
    glBindFramebuffer(GL_FRAMEBUFFER, renderTarget.mainFB);
    GL_UseMaterial(material, shaderId, assets);
    shader.SetVec4("_SIZE", sizeScaleVec);

    i32 model_Location = glGetUniformLocation(shaderId, ID_UNIFORM_MODEL);
    i32 view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
    i32 proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);
    i32 color_Location = glGetUniformLocation(shaderId, ID_COLOR_TINT);
    i32 scaleLoc = glGetUniformLocation(shaderId, "_LOD_SCALE");
    i32 skirtLoc = glGetUniformLocation(shaderId, "_IS_SKIRT");
    i32 camPosLoc = glGetUniformLocation(shaderId, "_CAMERA_POSITION");

    f32 terrainY = transform.position[1];
    Transform& camTransform = scene.transforms.GetItemRef(camera.transformHandle);
    f32 camx = camTransform.position[0];
    f32 camy = camTransform.position[1];
    f32 camz = camTransform.position[2];

    if (terrain.debugSnapping) {
        camx = terrain.testOffsetX;
        camz = terrain.testOffsetZ;
    }
    camx = terrain.testViewPositionX;
    camz = terrain.testViewPositionZ;

    // terrainY = 0.0f;
    vec2 viewPosXZ = {0, 0};
    f32 yMax = FLT_MAX / 2.0f;
    f32 yMin = -yMax;

    auto DrawMesh = [&](ClipmapMesh& mesh, f32 ox, f32 oz,
        f32 scale, f32 skirt
#ifdef DRAW_DEBUG_TERRAIN_COLORS
        , vec4 color
#endif
        )
    {
        vec3 aabbMin = {ox, yMin, oz};
        vec3 aabbMax = {ox + mesh.sizeX * scale, yMax, oz + mesh.sizeZ * scale};
        bool culled = !IsInsideFrustum(aabbMin, aabbMax, frustumPlanes);
        if (culled) {
            return;
        }

        mat4 matModel;
        glm_mat4_identity(matModel);
        glm_translate(matModel, vec3{ox, terrainY, oz});
        glm_scale(matModel, vec3{scale, 1.0f, scale});
        glUniformMatrix4fv(model_Location, 1, GL_FALSE, (f32*)matModel);
        glUniformMatrix4fv(view_Location, 1, GL_FALSE, (f32*)camera.viewMatrix);
        glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (f32*)camera.projectionMatrix);
        glUniform1f(scaleLoc, scale);
        glUniform1f(skirtLoc, skirt);

        glUniform2fv(camPosLoc, 1, (const f32*)viewPosXZ);
#ifdef DRAW_DEBUG_TERRAIN_COLORS
        glUniform4fv(color_Location, 1, (const f32*)color);
#endif
        glBindVertexArray(mesh.vao);
        glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);
    };


#ifdef DRAW_DEBUG_TERRAIN_COLORS
    vec4 color1 = {0.65, 0.52, 0.78255, 1.0};
    vec4 color2 = {0.65, 1.0, 0.85, 1.0};
    vec4 color3 = {0.0, 1.0, 1.0, 1.0};
    vec4 color4 = {0.25f, 0.25f, 0.25f, 1.0};
    vec4 color5 = {0.5, 0.5, 1.0, 1.0};
#endif

    f32 R = terrain.R;
    f32 M = terrain.M;
    f32 Gap = terrain.Gap;
    i32 LODS = terrain.LODS;
    // Central chunk
    {
        viewPosXZ[0] = static_cast<f32>( static_cast<i32>(camx) );
        viewPosXZ[1] = static_cast<f32>( static_cast<i32>(camz) );
        f32 offset = std::floor(-R * 0.5f);
        f32 x = viewPosXZ[0] + offset;
        f32 z = viewPosXZ[1] + offset;
#ifdef DRAW_DEBUG_TERRAIN_COLORS
        DrawMesh(terrain.centerMesh, x, z, 1.0f, 0.0f, color1);
#else
        DrawMesh(terrain.centerMesh, x, z, 1.0f, 0.0f);
#endif
    }

    // Rings around center starting from scale = 1
    for (i32 i = 0; i < LODS; i++) {

        i32 x0, x1, x2, x3, xgap = 0.0;
        i32 z0, z1, z2, z3, zgap = 0.0;
        // Coordinates for vertical trims
        i32 trimVertL, trimVertR, trimVertZ = 0;
        // Coordinates for horizontal trims
        i32 trimHorLow, trimHorHigh, trimHorX = 0;

        // Coordinates for vertical skirts
        i32 skirtVertLeft = 0, skirtVertRight = 0, skirtVertZ = 0;
        // Coordinates for horizontal skirts
        i32 skirtZLow = 0, skirtZHigh = 0, skirtHorX = 0;

        i32 scale = static_cast<i32>(1 << i);
        i32 stepsX = (i32)(camx / scale);
        i32 stepsZ = (i32)(camz / scale);
        i32 camSnapX = stepsX * scale;
        i32 camSnapZ = stepsZ * scale;
        // if (frameCount % 120 == 0)
            // printf("level %i, stepsX %i, stepsZ %i, camSnapX %i, camSnapZ %i \n\n", i, stepsX, stepsZ, camSnapX, camSnapZ);

        viewPosXZ[0] = camSnapX;
        viewPosXZ[1] = camSnapZ;

        i32 r = static_cast<i32>(R * scale);
        i32 m = static_cast<i32>(M * scale);
        i32 gap = static_cast<i32>(Gap * scale);

        x0 = std::floor(-r) + camSnapX;
        z0 = std::floor(-r) + camSnapZ;

        x1 = x0 + m;
        xgap = x1 + m;
        x2 = xgap + gap;
        x3 = x2 + m;

        z1 = z0 + m;
        zgap = z1 + m;
        z2 = zgap + gap;
        z3 = z2 + m;

        // region Trims Coordinate Calculation
        trimVertZ = z0 - scale; // constant for both vertical trims
        trimHorX = x0; // constant for both horizontal trims


        if (stepsX >= 0) {
            trimVertL = (x0 - scale);
            skirtVertLeft = trimVertL;

            if (stepsX % 2 == 0) {
                trimVertR = (x3 + m);
                skirtVertRight = trimVertR + scale;
            }
            else {
                trimVertR = trimVertL - scale;
                skirtVertRight = trimVertR;
            }
        }
        else {
            trimVertR = (x3 + m);
            skirtVertRight = trimVertR + scale;

            if (stepsX % 2 != 0) {
                trimVertL = trimVertR + scale;
                skirtVertLeft = trimVertL + scale;
            }
            else {
                trimVertL = x0 - scale;
                skirtVertLeft = trimVertL;
            }
        }

        if (stepsZ >= 0) {
            trimHorLow = (z0 - scale);
            skirtZLow = trimHorLow;

            if (stepsZ % 2 == 0) {
                trimHorHigh = (z3 + m);
                skirtZHigh = trimHorHigh + scale;
            }
            else {
                trimHorHigh = trimHorLow - scale;
                skirtZHigh  = trimHorHigh;
                trimVertZ -= scale;
            }
        }
        else {
            trimHorHigh = (z3 + m);
            skirtZHigh = trimHorHigh + scale;
            if (stepsZ % 2 != 0) {
                trimHorLow = trimHorHigh + scale;
                skirtZLow = trimHorLow + scale;
                trimVertZ += scale;
            }
            else {
                trimHorLow = z0 - scale;
                skirtZLow = trimHorLow;
            }
        }
        skirtHorX = trimHorX - scale;
        skirtVertZ = trimVertZ;

        // printf("LOD lvl %d\n", i);
        // printf("skirtVertLeft: %i\n", skirtVertLeft);
        // printf("skirtVertRight: %i\n", skirtVertRight);
        // printf("skirtVertZ: %i\n", skirtVertZ);
        // printf("skirtZLow: %i\n", skirtZLow);
        // printf("skirtZHigh: %i\n", skirtZHigh);
        // printf("skirtHorX: %i\n", skirtHorX);


//endregion

// region Drawing
#ifdef DRAW_DEBUG_TERRAIN_COLORS
        DrawMesh(terrain.blockMesh, x0, z0, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x1, z0, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x2, z0, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x3, z0, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x0, z1, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x3, z1, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x0, z2, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x3, z2, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x0, z3, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x1, z3, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x2, z3, scale, 0.0f, color4);
        DrawMesh(terrain.blockMesh, x3, z3, scale, 0.0f, color4);
        DrawMesh(terrain.fixUpXMesh, x0, zgap, scale, 0.0f, color2);
        DrawMesh(terrain.fixUpXMesh, x3, zgap, scale, 0.0f, color2);
        DrawMesh(terrain.fixUpZMesh, xgap, z0, scale, 0.0f, color3);
        DrawMesh(terrain.fixUpZMesh, xgap, z3, scale, 0.0f, color3);

        DrawMesh(terrain.trimZMesh, trimVertL, trimVertZ, scale, 0.0f, color5);
        DrawMesh(terrain.trimZMesh, trimVertR, trimVertZ, scale, 0.0f, color5);
        DrawMesh(terrain.trimXMesh, trimHorX, trimHorLow, scale, 0.0f, color5);
        DrawMesh(terrain.trimXMesh, trimHorX, trimHorHigh, scale, 0.0f, color5);

        DrawMesh(terrain.zeroAreaX1, skirtHorX, skirtZHigh, scale, 1.0f, color3);
        DrawMesh(terrain.zeroAreaX2, skirtHorX, skirtZLow, scale, 1.0f, color3);
        DrawMesh(terrain.zeroAreaZ1, skirtVertLeft, skirtVertZ, scale, 1.0f, color3);
        DrawMesh(terrain.zeroAreaZ2, skirtVertRight, skirtVertZ, scale, 1.0f, color3);


#else
        DrawMesh(terrain.blockMesh, x0, z0, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x1, z0, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x2, z0, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x3, z0, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x0, z1, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x3, z1, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x0, z2, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x3, z2, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x0, z3, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x1, z3, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x2, z3, scale, 0.0f);
        DrawMesh(terrain.blockMesh, x3, z3, scale, 0.0f);
        DrawMesh(terrain.fixUpXMesh, x0, zgap, scale, 0.0f);
        DrawMesh(terrain.fixUpXMesh, x3, zgap, scale, 0.0f);
        DrawMesh(terrain.fixUpZMesh, xgap, z0, scale, 0.0f);
        DrawMesh(terrain.fixUpZMesh, xgap, z3, scale, 0.0f);
        DrawMesh(terrain.trimZMesh, trimVertL, trimVertZ, scale, 0.0f);
        DrawMesh(terrain.trimZMesh, trimVertR, trimVertZ, scale, 0.0f);
        DrawMesh(terrain.trimXMesh, trimHorX, trimHorLow, scale, 0.0f);
        DrawMesh(terrain.trimXMesh, trimHorX, trimHorHigh, scale, 0.0f);

        DrawMesh(terrain.zeroAreaX1, skirtHorX, skirtZHigh, scale, 1.0f);
        DrawMesh(terrain.zeroAreaX2, skirtHorX, skirtZLow, scale, 1.0f);
        DrawMesh(terrain.zeroAreaZ1, skirtVertLeft, skirtVertZ, scale, 1.0f);
        DrawMesh(terrain.zeroAreaZ2, skirtVertRight, skirtVertZ, scale, 1.0f);
#endif

//endregion
    }

    // Reset State
    glBindTexture(GL_TEXTURE_2D, 0);
    glBindVertexArray(0);
    glUseProgram(0);
}

//endregion


void GL_TransparentPass(GameScene& scene,
                        Camera& camera,
                        AssetManager& assets) {
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);
}


// region Post-Processing
void SSR(const RenderTarget& target,
         const PostProcessStack& stack,
         Camera& camera,
         AssetManager& assets) {

    GL_PrepareRenderFrameBufferToScreen(stack.postProcessB_FB);

    Shader& shaderSSR = assets.GetShader(assets.shaderSSR);
    auto ssrShaderId = shaderSSR.GetShaderId();
    glUseProgram(ssrShaderId);

    mat4 inverseProjection;
    glm_mat4_inv(camera.projectionMatrix, inverseProjection);
    shaderSSR.SetMat4(ID_UNIFORM_VIEW, camera.viewMatrix);
    shaderSSR.SetMat4(ID_UNIFORM_PROJECTION, camera.projectionMatrix);
    shaderSSR.SetMat4(ID_UNIFORM_PROJECTION_INVERSE, inverseProjection);

    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, stack.postProcessA_Texture);
    glUniform1i(glGetUniformLocation(ssrShaderId, ID_COLOR_TEX), 0);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, target.normalTexture);
    glUniform1i(glGetUniformLocation(ssrShaderId, ID_NORMALS_TEX), 1);

    glActiveTexture(GL_TEXTURE2);
    glBindTexture(GL_TEXTURE_2D, stack.postProcessA_DepthTex);
    glUniform1i(glGetUniformLocation(ssrShaderId, ID_DEPTH_TEX), 2);

    glUniform1f(glGetUniformLocation(ssrShaderId, ID_FAR_PLANE), camera.farPlane);
    glUniform1f(glGetUniformLocation(ssrShaderId, ID_NEAR_PLANE), camera.nearPlane);

    GL_RenderScreenQuad(target);
    // glBindVertexArray(target.quadVAO);
    // glDrawArrays(GL_TRIANGLES, 0, 6);
}


void Bloom(const RenderTarget& target,
           const PostProcessStack& stack,
           AssetManager& assets,
           bool readFromBufferA,

           f32 BloomExposure,
           f32 BloomThreshold) {


    auto sourceFrameTextureId =  readFromBufferA ? stack.postProcessA_Texture : stack.postProcessB_Texture;

    // Downsampling
    auto count = stack.bloomMipmaps.size();
    for (size_t pass = 0; pass < count; pass++) {
        auto& mip = stack.bloomMipmaps[pass];
        vec2 dim = {1.0f / mip.sizeX, 1.0f / mip.sizeY};
        GLuint shaderId{};

        glBindFramebuffer(GL_FRAMEBUFFER, mip.fbo);
        glViewport(0, 0, mip.sizeX, mip.sizeY);
        // USING DIFFERENT SHADER FOR 1st pass
        if (pass == 0) {
            auto& downSampleFShader = assets.GetShader(assets.shaderBloomDownsampleFirst);
            shaderId = downSampleFShader.GetShaderId();
            glUseProgram(shaderId);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, sourceFrameTextureId);

            downSampleFShader.SetTexture("u_InputTexture", 0);
            downSampleFShader.SetVec2("u_TexelSize", dim);
            downSampleFShader.SetFloat("u_Threshold", BloomThreshold);
        }
        else {
            auto& prevMip = stack.bloomMipmaps[pass - 1];
            Shader& downSampleShader = assets.GetShader(assets.shaderBloomDownsample);
            shaderId = downSampleShader.GetShaderId();
            glUseProgram(shaderId);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, prevMip.texture);
            downSampleShader.SetTexture("u_InputTexture", 0);
            downSampleShader.SetVec2("u_TexelSize", dim);
        }
        GL_RenderScreenQuad(renderTarget);
    }
    // Upsampling
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE);

        auto& upSampleShader = assets.GetShader(assets.shaderBloomUpSample);
        auto shaderId = upSampleShader.GetShaderId();
        glUseProgram(shaderId);
        upSampleShader.SetFloat("u_FilterRadius", 0.005f);

        for (i32 pass = count - 1; pass >= 1; pass--) {
            auto& mipSrc = stack.bloomMipmaps[pass];
            auto& mipTarget = stack.bloomMipmaps[pass - 1];

            glBindFramebuffer(GL_FRAMEBUFFER, mipTarget.fbo);
            glViewport(0, 0, mipTarget.sizeX, mipTarget.sizeY);

            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_2D, mipSrc.texture);
            upSampleShader.SetTexture("u_InputTexture", 0);

            GL_RenderScreenQuad(renderTarget);
        }
        glDisable(GL_BLEND);
    }

    // Composition
    {
        if (readFromBufferA)
            GL_PrepareRenderFrameBufferToScreen(stack.postProcessB_FB);
        else
            GL_PrepareRenderFrameBufferToScreen(stack.postProcessA_FB);

        glViewport(0, 0, target.width, target.height);
        auto& composeShader = assets.GetShader(assets.shaderBloomComposite);
        glUseProgram(composeShader.GetShaderId());

        composeShader.SetFloat("u_Exposure", BloomExposure);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, sourceFrameTextureId);
        glActiveTexture(GL_TEXTURE1);
        glBindTexture(GL_TEXTURE_2D, stack.bloomMipmaps[0].texture);

        GL_RenderScreenQuad(renderTarget);
    }
}


void GL_RenderPostProcess(const RenderTarget& target,
                          const PostProcessStack& stack,
                          AssetManager& assets,
                          Camera& camera,
                          const ProjectSettings& settings) {

    bool readBufferA = true;
    // SSR writes to B_Buffer
    if (settings.PostProcess_SSR) {
        SSR(target, stack, camera, assets);
        readBufferA = false;
    }
    // Bloom writes to A_Buffer
    if (settings.PostProcess_Bloom) {
        Bloom(target, stack, assets, readBufferA, settings.BloomExposure, settings.BloomThreshold);
        readBufferA = true;
    }

    // Final Screen output
    {
        GL_RenderPostProcessBufferToScreen(stack, settings, assets, readBufferA);
    }

    GL_CleanState();
}
//endregion



//region UI pass
void GL_UIPass(RenderTarget& target, GameScene& scene, AssetManager& assets, MyGui* gui) {
    constexpr i32 quadIndexCount = 6;

    imax count = scene.activeUIHandles.size();
    if (count == 0) {
        return;
    }
    vec2 screenSize = {(f32)target.width, (f32)target.height};
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_CULL_FACE);
    // glEnable(GL_CULL_FACE);
    // glCullFace(GL_FRONT);

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    for (size_t i = 0; i < count; i++) {
        Handle h = scene.activeUIHandles[i];
        auto& uiObj = scene.uiObjectsPool.GetItemRef(h);

        auto& material = assets.materials.GetItemRef(uiObj.hMaterial);
        auto& shader = assets.shaders.GetItemRef(material.shaderHandle);
        i32 shaderId = shader.GetShaderId();
        GL_UseMaterial(material, shaderId, assets);
        vec4 positionSize = {uiObj.position[0], uiObj.position[1], uiObj.size[0], uiObj.size[1]};
        shader.SetVec4("u_PositionSize", positionSize);
        shader.SetVec2("u_ScreenResolution", screenSize);
        //
        // printf("----------------\n\n");
        // GL_BindMaterialTextures(material, assets, true);
        // printf("----------------\n\n");

        glBindVertexArray(uiObj.vao);
        glDrawElements(GL_TRIANGLES, quadIndexCount, GL_UNSIGNED_INT, nullptr);
    }

    if (gui != nullptr) {
        gui->RenderFrame();
    }

    GL_CleanState();
    glDisable(GL_BLEND);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
}
//endregion




void GL_RenderScene(Engine& engine) {

    BindRenderTarget(renderTarget);
    auto& settings = engine.settings;
    auto& camera = engine.scene.camera;
    auto& assets = engine.assetManager;
    bool postProcess = settings.PostProcess;
    bool devDepths = settings.DevRenderDepths;
    bool devNormals = settings.DevRenderNormals;
    bool devUvs = settings.DevRenderUvs;
    bool pp_ssr = settings.PostProcess_SSR;
    bool pp_bloom = settings.PostProcess_Bloom;
    bool pp_dof = settings.PostProcess_DOF;
    bool pp_ssao = settings.PostProcess_SSAO;
    bool pp_bw = settings.PostProcess_BW;

    if (devDepths || devNormals || devUvs) {
        postProcess = false;
    }

#ifdef DEBUG_FLAGS
    std::cout << "postProcess: " << postProcess << ", "; std::cout << "devDepths: " << devDepths << ", "; std::cout << "devNormals: " << devNormals << ", "; std::cout << "devUvs: " << devUvs << ", "; std::cout << "devUvs: " << devUvs << ", "; std::cout << "pp_ssr: " << pp_ssr << ", "; std::cout << "pp_bloom: " << pp_bloom << ", "; std::cout << "pp_dof: " << pp_dof << ", "; std::cout << "pp_ssao: " << pp_ssao << ", "; std::cout << "pp_bw: " << pp_bw << ", "; std::cout << std::endl;
#endif

    auto& scene = engine.scene;
    glm_vec3_copy(settings.Ambient_Light_Color, scene.ambientLightColor);
    glm_vec3_copy(settings.Direct_Light_Color, scene.mainLight.color);
    scene.ambientIntensity = settings.Ambient_Light_Brightness;
    scene.mainLight.intensity = settings.Direct_Light_Intensity;

    GL_OpaquePass(scene, camera, assets, settings);
    GL_InstancedPass(renderTarget, assets, camera);
    GL_TerrainPass(renderTarget, scene, camera, assets);
    GL_DeferredLightingPass(renderTarget, ppStack, scene, camera, assets);
    GL_BlitDepthFromGBuffer(renderTarget, ppStack);

    if (settings.RenderSkyBox) {
        GL_SkyboxPass(scene, assets, ppStack.postProcessA_FB);
    }
    // Transparent Pass
    // GL_ForwardRenderTransparent(engine.scene, camera, assets);
    GL_RenderPostProcess(renderTarget, ppStack, assets, camera, settings);
    GL_UIPass(renderTarget, scene, assets, &engine.gui);

    // if (devDepths) {
    //     GL_RenderDepthOnly(renderTarget, assets.GetShader(assets.shaderDepthOnly), camera);
    // }
    // else if (devNormals) {
    //     GL_RenderNormalsOnly(renderTarget, assets.GetShader(assets.shaderNormalsOnly), camera);
    // }
    // else if (postProcess) {
    //     GL_RenderPostProcess(renderTarget, ppStack, assets, camera, settings);
    // }
    // else {
    //      //Main Render Image without effects
    //      RenderColorBufferToScreen(renderTarget, assets.GetScreenRenderTextureShader());
    // }
    GL_CleanState();

}

// endregion
