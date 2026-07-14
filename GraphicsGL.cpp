#include "GraphicsGL.h"
#include <iostream>
#include <ostream>
#include "Material.h"
#include "Uniforms.h"
#include "Camera.h"
#include "Engine.h"

class Engine;
static int DidLogTransforms;

struct RenderTarget {
    GLuint fbo;
    GLuint colorTexture;
    GLuint rbo;
    GLuint quadVAO;
    GLuint quadVBO;
};

static RenderTarget renderTarget = {};

// Forward-declaration
void RenderScreenTexture(const RenderTarget& target, Shader& quadShader);


void GL_ResizeRenderTarget(i32 width, i32 height) {
    RenderTarget& target = renderTarget;
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);

    glBindRenderbuffer(GL_RENDERBUFFER, target.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

    glBindTexture(GL_TEXTURE_2D, 0);
    glBindRenderbuffer(GL_RENDERBUFFER, target.rbo);

    glViewport(0, 0, width, height);
}

void GL_InitMaterialParametersDefault3D(Material& material) {
    material.shaderName = "Default3D";
    const float shade = 0.96f;
    material.SetVectorDefinition(ID_COLOR_TINT, {shade, shade, shade, 1.0});

    material.SetFloatDefinition(ID_SMOOTHNESS, .25f);
    material.SetFloatDefinition(ID_METALLIC, .25f);

    material.SetTextureDefinition(ID_BASE_MAP, "default_white");
    material.SetTextureDefinition(ID_NORMAL_MAP, "default_normal");
    material.SetTextureDefinition(ID_SKYBOX_CUBEMAP, "Skybox");

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
        material.SetTextureDefinition(ID_SKYBOX_CUBEMAP, "Skybox");
        material.SetFloatDefinition(ID_SKYBOX_BRIGHTNESS, 0.5f);
    }
}


void GL_BuildScreenRenderQuad(RenderTarget& target, i32 width, i32 height) {
    float quadVertices[] = {
        // positions   // uvs
        -1.0f,  1.0f,  0.0f, 1.0f,
        -1.0f, -1.0f,  0.0f, 0.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
        -1.0f,  1.0f,  0.0f, 1.0f,
         1.0f, -1.0f,  1.0f, 0.0f,
         1.0f,  1.0f,  1.0f, 1.0f
    };

    glGenVertexArrays(1, &target.quadVAO);
    glGenBuffers(1, &target.quadVBO);

    glBindVertexArray(target.quadVAO);

    glBindBuffer(GL_ARRAY_BUFFER, target.quadVBO);
    glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
    // Vertices
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(0 * sizeof(float) ));
    // UVs
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (const void*)(2 * sizeof(float) ));

    // Create texture for the FB
    glGenTextures(1, &target.colorTexture);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    // Create framebuffer
    glGenFramebuffers(1, &target.fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, target.colorTexture, 0);
    // Create render-buffer for depth and stencil
    glGenRenderbuffers(1, &target.rbo);
    glBindRenderbuffer(GL_RENDERBUFFER, target.rbo);
    glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
    glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, target.rbo);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!\n";
    }
    // unbind into default state
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}


void GL_InitGraphics(i32 width, i32 height) {
    std::cout << "[Graphics] Init " << width << "height: " << height << std::endl;
    GL_BuildScreenRenderQuad(renderTarget, width, height);
}


void GL_UpdateBackground(GameScene& scene) {
}

void GL_InitMaterialProperties(Material& material, AssetManager& assets) {
    material.didInit = true;
    std::cout << "[Graphics][MaterialProperties] Shader: " << material.shaderName << std::endl;
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
        std::cout << "Material.Float] [shader " << shaderId << "] Name: " << paramName << ", uniformLocation: " << uniformLocation << std::endl;
        material.floats.emplace_back(uniformLocation, material.floatsDefinitions[i].second);
    }

    for (auto i = 0; i < material.vectorsDefinitions.size(); i++) {
        auto& paramName = material.vectorsDefinitions[i].first;

        u32 uniformLocation = glGetUniformLocation(shaderId, paramName.c_str());
        std::cout << "[Material.Vector] [shader " << shaderId << "] Name: " << paramName << ", uniformLocation: " << uniformLocation << std::endl;

        material.vectors.emplace_back(uniformLocation, material.vectorsDefinitions[i].second);
    }

    for (auto i = 0; i < material.texturesDefinitions.size(); i++) {
        assets.GetDefault3D();
        auto& paramName = material.texturesDefinitions[i].first;
        auto& assetName = material.texturesDefinitions[i].second;

        Handle textureHandle = {};
        if (assetName.empty()){
            textureHandle.Copy(assets.defaultWhiteTexture);
        }
        else {
            textureHandle = assets.FindTextureByName(assetName.c_str());
        }

        if (textureHandle.IsEmpty()) {
            textureHandle.Copy(assets.defaultWhiteTexture);
            if (!assetName.empty()) { // Empty name is intentional for DefaultTexture, so don't log in that case
                std::cerr << "Failed to find texture: '" << assetName << "'" << std::endl;
            }
        }
        u32 uniformLocation = glGetUniformLocation(shaderId, paramName.c_str());
        material.textures.emplace_back(uniformLocation, textureHandle);

        Texture& txt = assets.textures.GetItemRef(textureHandle);
        std::cout << "[Material.Texture] [shader " << shaderId << "] Param: " << paramName << " Asset: " << assetName << " uniformLocation: " << uniformLocation << std::endl;

    }
    glUseProgram(0);

    std::cout << std::endl << std::endl;
}


void GL_AllocateGraphicsSkybox(RenderSubMesh& obj) {
    float skyboxVertices[] = {
        -1.0f,  1.0f, -1.0f,  -1.0f, -1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,   1.0f,  1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f, -1.0f, -1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f,  1.0f, -1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
         1.0f, -1.0f, -1.0f,   1.0f, -1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f,  1.0f, -1.0f,   1.0f, -1.0f, -1.0f,
        -1.0f, -1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,   1.0f, -1.0f,  1.0f,  -1.0f, -1.0f,  1.0f,
        -1.0f,  1.0f, -1.0f,   1.0f,  1.0f, -1.0f,   1.0f,  1.0f,  1.0f,
         1.0f,  1.0f,  1.0f,  -1.0f,  1.0f,  1.0f,  -1.0f,  1.0f, -1.0f,
        -1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f, -1.0f,
         1.0f, -1.0f, -1.0f,  -1.0f, -1.0f,  1.0f,   1.0f, -1.0f,  1.0f
    };
    glGenVertexArrays(1, &obj.vao);
    glGenBuffers(1, &obj.vbo);
    glBindVertexArray(obj.vao);
    glBindBuffer(GL_ARRAY_BUFFER, obj.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(skyboxVertices), &skyboxVertices, GL_STATIC_DRAW);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
    glBindVertexArray(0);
}

void GL_AllocateGraphicsForObject(RenderObject& obj, AssetManager& assets)
{
    obj.subMeshses.reserve(1);
    if (obj.subMeshses.size() < 1) {
        obj.subMeshses.resize(1);
    }
    for (auto& meshRenderData : obj.subMeshses) {

        Mesh& mesh = assets.meshes.GetItemRef(meshRenderData.hMesh);

        GLuint vao;
        GLuint vbo;
        GLuint ebo;

        glGenVertexArrays(1, &vao);
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);
        glBindVertexArray(vao);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(float) * mesh.vertexDataCount, mesh.vertexData, GL_DYNAMIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(int) * mesh.indexCount, mesh.indexData, GL_DYNAMIC_DRAW);

        int stride = mesh.stride * sizeof(float);
        int attributeIdx = 0;
        glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)0); // XYZ
        glEnableVertexAttribArray(attributeIdx);
        attributeIdx++;
        if (mesh.startIndexUV >= 0) {
            glVertexAttribPointer(attributeIdx, 2, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexUV*sizeof(float))); // UV
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }
        if (mesh.startIndexNormals >= 0) {
            glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexNormals*sizeof(float))); // XYZ normals
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }
        if (mesh.startIndexColor >= 0) {
            glVertexAttribPointer(attributeIdx, 4, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexColor*sizeof(float))); // RGBA
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }
        if (mesh.startIndexTangents >= 0) { // Tangents
            glVertexAttribPointer(attributeIdx, 3, GL_FLOAT, GL_FALSE, stride, (const void*)(mesh.startIndexColor*sizeof(float))); // RGBA
            glEnableVertexAttribArray(attributeIdx);
            attributeIdx++;
        }

        glBindVertexArray(0);
        meshRenderData.vao = vao;
        meshRenderData.vbo = vbo;
        meshRenderData.ebo = ebo;
    }
}


static int DBG_LVL = 0;
static int MAX_DBG = 0;

static int logTimes = 0;
static int MAX_LOG = 2;

void BindRenderTarget(const RenderTarget& target) {
    glBindFramebuffer(GL_FRAMEBUFFER, target.fbo);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
}





void GL_ForwardRenderOpaques(GameScene& scene, Camera& camera, AssetManager& assets, ProjectSettings& settings) {
    DBG_LVL++;

    vec4& backgroundColor = scene.backgroundColor;
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glDepthMask(GL_TRUE);

    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);
    glDisable(GL_BLEND); // no blending here

    if (settings.Gamma_Correction) {
        glEnable(GL_FRAMEBUFFER_SRGB);
    }
    else {
        glDisable(GL_FRAMEBUFFER_SRGB);
    }

    mat4 viewProjMatrix;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProjMatrix);

    scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    Transform& cameraTransform = scene.transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);


    auto& shaders = assets.shaders.GetVector();
    for (auto& shader : shaders) {
        if (!shader.GetAcceptsLighting()) {
            continue;
        }

        auto shaderGL = shader.GetShaderId();
        glUseProgram(shaderGL);

        shader.SetVec3(ID_UNIFORM_VIEW_POS, cameraTransform.position);
        shader.SetVec3(ID_UNIFORM_AMBIENT_LIGHT_COLOR, scene.ambientLightColor);
        shader.SetFloat(ID_UNIFORM_AMBIENT_LIGHT_INTENSITY, scene.ambientIntensity);
        shader.SetVec3("DIRECTIONAL_LIGHT.direction", mainLightDir);
        shader.SetVec3("DIRECTIONAL_LIGHT.color", scene.mainLight.color);
        shader.SetFloat("DIRECTIONAL_LIGHT.intensity", scene.mainLight.intensity);
    }

    for (auto& objHandle : scene.existingObjects) {
        if (scene.renderObjects.IsValid(objHandle) == false) {
            continue;
        }
        RenderObject& obj = scene.renderObjects.GetItemRef(objHandle);
        Transform& transform = scene.transforms.GetItemRef(obj.hTransform);

        if (DidLogTransforms < 2) {
        }

        mat4 mvp;
        glm_mat4_mul(viewProjMatrix, transform.modelMatrix, mvp);

        for (auto& renderData : obj.subMeshses) {

            Material& material = assets.materials.GetItemRef(renderData.hMaterial);
            Shader& shader = assets.GetShader(material.shaderHandle);
            bool isNull = assets.shaders.IsNullItem(shader);
            // printf("+ shader handle %d, %d. NULL? %d \n", material.shaderHandle.index, material.shaderHandle.generation, ((int)isNull));

            if (isNull) {
                continue;
            }
            Mesh& mesh = assets.meshes.GetItemRef(renderData.hMesh);
            auto shaderId = shader.GetShaderId();
            glUseProgram(shaderId);

            int model_Location = glGetUniformLocation(shaderId, ID_UNIFORM_MODEL);
            int view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
            int proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);

            glUniformMatrix4fv(model_Location, 1, GL_FALSE, (float*)transform.modelMatrix);
            glUniformMatrix4fv(view_Location, 1, GL_FALSE, (float*)camera.viewMatrix);
            glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (float*)camera.projectionMatrix);

            for (auto floatPair : material.floats) {
                glUniform1f(floatPair.first, floatPair.second);
            }
            for (auto vecPair : material.vectors) {
                vec4 vec;
                vec[0] = vecPair.second.x;
                vec[1] = vecPair.second.y;
                vec[2] = vecPair.second.z;
                vec[3] = vecPair.second.w;
                glUniform4fv(vecPair.first, 1, (const float*)vec);
            }
            u32 textureNumber = 0;
            for (auto texturePair : material.textures) {

                auto& texture = assets.textures.GetItemRef(texturePair.second);
                auto& texParamName = texturePair.first;

                if (assets.textures.IsNullItem(texture)) {
                    continue;
                }
                glActiveTexture(GL_TEXTURE0 + textureNumber);
                auto format = texture.pixelFormat == 0 ? GL_TEXTURE_2D : GL_TEXTURE_CUBE_MAP;
                glBindTexture(format, texture.glHandle);
                glUniform1i(texParamName, textureNumber);
                textureNumber++;
              //  if (DBG_LVL < MAX_DBG) {
              //      printf("Texture Handle %d, %d\n", texturePair.second.index, texturePair.second.generation);
              //      printf("Texture GL handle %d\n", texture.glHandle);
              //      printf("Texture Name: %s\n" ,texture.name);
              //  }
            }

            glBindVertexArray(renderData.vao);
            glDrawElements(GL_TRIANGLES, mesh.indexCount, GL_UNSIGNED_INT, nullptr);

            // Reset State
            glBindTexture(GL_TEXTURE_2D, 0);
            glBindVertexArray(0);
            glUseProgram(0);
        }
    }

    glBindVertexArray(0);
    glUseProgram(0);
}

void ClearBackgroundNoSkyBox() {

}


void GL_RenderSkybox(GameScene& scene, AssetManager& assets) {
    logTimes++;

    auto& skybox = scene.skybox;
    auto& material = assets.materials.GetItemRef(skybox.renderData.hMaterial);
    auto& camera = scene.camera;
    auto& shader = assets.shaders.GetItemRef(material.shaderHandle);
    auto shaderId = shader.GetShaderId();
    if (logTimes < MAX_LOG) {

        std::cout << std::endl << "GL_RenderSkybox " << "Shader handle (" << material.shaderHandle.index  << ")" << std::endl;
    }

    mat4 viewNoTranslation;
    glm_mat4_copy(camera.viewMatrix, viewNoTranslation);
    viewNoTranslation[3][0] = 0.0f;
    viewNoTranslation[3][1] = 0.0f;
    viewNoTranslation[3][2] = 0.0f;

    glDepthFunc(GL_LEQUAL);
    glUseProgram(shaderId);

    if (logTimes < MAX_LOG) {
        std::cout << "material.textures count:  " << material.textures.size() << std::endl;
        for (auto& texture : material.textures) {
            std::cout << "Texture --  " << texture.first<< ", " << "Handle: " << texture.second.index << ", " << texture.second.generation << std::endl;
        }
    }

    if (!material.textures.empty()) {

        auto& texturePair = material.textures[0];
        auto& texture = assets.textures.GetItemRef(texturePair.second);
        if (!assets.textures.IsNullItem(texture)) {
            glActiveTexture(GL_TEXTURE0);
            glBindTexture(GL_TEXTURE_CUBE_MAP, texturePair.first);
            if (logTimes < MAX_LOG) {
            }

        }
        else {
            // error
        }
    }

    auto loc = glGetUniformLocation(shaderId, ID_SKYBOX_CUBEMAP);
    glUniform1i(loc, 0);

    glUniformMatrix4fv(glGetUniformLocation(shaderId, ID_UNIFORM_VIEW), 1, GL_FALSE, (float*)viewNoTranslation);
    glUniformMatrix4fv(glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION), 1, GL_FALSE, (float*)camera.projectionMatrix);

    for (auto floatPair : material.floats) {
        glUniform1f(floatPair.first, floatPair.second);
    }
    for (auto vecPair : material.vectors) {
        vec4 vec;
        vec[0] = vecPair.second.x;
        vec[1] = vecPair.second.y;
        vec[2] = vecPair.second.z;
        vec[3] = vecPair.second.w;
        glUniform4fv(vecPair.first, 1, (const float*)vec);
    }

    glBindVertexArray(skybox.renderData.vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glUseProgram(0);
}


void GL_ForwardRenderTransparent(GameScene& scene, Camera& camera, AssetManager& assets) {

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glDepthMask(GL_FALSE);


}



void RenderScreenTexture(const RenderTarget& target, Shader& quadShader) {
    glBindFramebuffer(GL_FRAMEBUFFER, 0); // default framebuffer

    glDisable(GL_BLEND);
    glDepthMask(GL_TRUE);

    glDisable(GL_DEPTH_TEST);
    glDisable(GL_STENCIL_TEST);
    glDisable(GL_CULL_FACE);

    glClearColor(1.0, 1.0, 1.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);

    auto shaderId = quadShader.GetShaderId();

    // std::string name = std::string(quadShader.GetName());
    // std::cout << "USING SHADER FOR QUAD: " <<shaderId << " Name: " << name << std::endl;

    glUseProgram(shaderId);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, target.colorTexture);
    glUniform1i(glGetUniformLocation(shaderId, ID_UNIFORM_SCREEN_TEXTURE), 0);
    glBindVertexArray(target.quadVAO);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    // cleanup
    glBindVertexArray(0);
    glUseProgram(0);
}




void GL_RenderScene(Engine& engine) {
    BindRenderTarget(renderTarget);

    glm_vec3_copy(engine.settings.Ambient_Light_Color, engine.scene.ambientLightColor);
    glm_vec3_copy(engine.settings.Direct_Light_Color, engine.scene.mainLight.color);
    engine.scene.ambientIntensity = engine.settings.Ambient_Light_Brightness;
    engine.scene.mainLight.intensity = engine.settings.Direct_Light_Intensity;


    GL_ForwardRenderOpaques(engine.scene, engine.scene.camera, engine.assetManager, engine.settings);
    if (engine.settings.RenderSkyBox) {
        GL_RenderSkybox(engine.scene, engine.assetManager);
    }
    else {
        ClearBackgroundNoSkyBox();
    }

    // GL_ForwardRenderTransparent(engine.scene, engine.scene.camera, engine.assetManager);

    RenderScreenTexture(renderTarget, engine.assetManager.GetScreenRenderTextureShader());
}
