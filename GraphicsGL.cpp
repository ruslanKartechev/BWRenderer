#include "GraphicsGL.h"
#include <iostream>
#include <ostream>
#include "Material.h"
#include "Uniforms.h"
#include "Camera.h"

static int DidLogTransforms;

void GL_UpdateBackground(GameScene scene) {
    vec4& backgroundColor = scene.backgroundColor;
    glClearColor(backgroundColor[0], backgroundColor[1], backgroundColor[2], backgroundColor[3]);
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);
}

void GL_InitMaterialProperties(Material& material, AssetManager& assets) {
    material.didInit = true;
    std::cout << "[Graphics] Looking for shader " << material.shaderName << std::endl;
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
        std::cout << "[Property Float] [shader " << shaderId << "] Name: " << paramName << ", uniformLocation: " << uniformLocation << std::endl;
        material.floats.emplace_back(uniformLocation, material.floatsDefinitions[i].second);
    }

    for (auto i = 0; i < material.vectorsDefinitions.size(); i++) {
        auto& paramName = material.vectorsDefinitions[i].first;

        u32 uniformLocation = glGetUniformLocation(shaderId, paramName.c_str());
        std::cout << "[Property Vector] [shader " << shaderId << "] Name: " << paramName << ", uniformLocation: " << uniformLocation << std::endl;

        material.vectors.emplace_back(uniformLocation, material.vectorsDefinitions[i].second);
    }

    for (auto i = 0; i < material.texturesDefinitions.size(); i++) {
        assets.GetDefault3D();
        auto& paramName = material.texturesDefinitions[i].first;
        auto& assetName = material.texturesDefinitions[i].second;

        Handle textureHandle = {};
        if (assetName.empty()){
            printf("Default white  handle: %d, %d \n", assets.defaultWhiteTexture.index, assets.defaultWhiteTexture.generation);
            textureHandle.Copy(assets.defaultWhiteTexture);
            printf("HANDLE AFTER COPY: %d, %d \n", textureHandle.index, textureHandle.generation);

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
        printf("Texture handle: %d, Name: %s, Loaded: %d \n", txt.glHandle, txt.name, txt.isLoaded);
        printf("[Property Texture] handle %d, %d \n",  textureHandle.index, textureHandle.generation);
        std::cout << "[Property Texture] [shader " << shaderId << "] Param: " << paramName << " Asset: " << assetName << " uniformLocation: " << uniformLocation << std::endl;

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

void GL_AllocateGraphicsForObject(RenderObject& obj, GameScene& scene)
{
    obj.meshData.reserve(1);
    if (obj.meshData.size() < 1) {
        obj.meshData.resize(1);
    }
    for (auto& meshRenderData : obj.meshData) {

        Mesh& mesh = scene.meshes.GetItemRef(meshRenderData.hMesh);

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

        glBindVertexArray(0);
        meshRenderData.vao = vao;
        meshRenderData.vbo = vbo;
        meshRenderData.ebo = ebo;
    }
}


static int DBG_LVL = 0;
static int MAX_DBG = 0;

void GL_ForwardRenderOpaques(GameScene& scene, Camera& camera, AssetManager& assets) {
    DBG_LVL++;
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    glEnable(GL_CULL_FACE);
    glCullFace(GL_FRONT);

    mat4 viewProjMatrix;
    glm_mat4_mul(camera.projectionMatrix, camera.viewMatrix, viewProjMatrix);

    scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    Transform& cameraTransform = scene.transforms.GetItemRef(camera.transformHandle);
    Transform& globalLightTransform = scene.transforms.GetItemRef(scene.mainLight.transformHandle);
    vec3 cameraViewDir;
    vec3 mainLightDir;
    Transform_GetFrw(cameraTransform, cameraViewDir);
    Transform_GetFrw(globalLightTransform, mainLightDir);

    {
        Shader& def3D = assets.GetDefault3D();
        glUseProgram(def3D.GetShaderId());
        def3D.SetVec3(ID_UNIFORM_VIEW_POS, cameraTransform.position);
        def3D.SetVec3(ID_UNIFORM_AMBIENT_LIGHT_COLOR, scene.ambientLightColor);
        def3D.SetFloat(ID_UNIFORM_AMBIENT_LIGHT_INTENSITY, scene.ambientIntensity);
        def3D.SetVec3("DIRECTIONAL_LIGHT.direction", mainLightDir);
        def3D.SetVec3("DIRECTIONAL_LIGHT.color", scene.mainLight.color);
        def3D.SetFloat("DIRECTIONAL_LIGHT.intensity", scene.mainLight.intensity);
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

        for (auto& renderData : obj.meshData) {

            Material& material = assets.materials.GetItemRef(renderData.hMaterial);
            Shader& shader = assets.GetShader(material.shaderHandle);
            bool isNull = assets.shaders.IsNullItem(shader);
            // printf("+ shader handle %d, %d. NULL? %d \n", material.shaderHandle.index, material.shaderHandle.generation, ((int)isNull));

            if (isNull) {
                continue;
            }
            Mesh& mesh = scene.meshes.GetItemRef(renderData.hMesh);
            auto shaderId = shader.GetShaderId();
            glUseProgram(shaderId);

            int model_Location = glGetUniformLocation(shaderId, ID_UNIFORM_MODEL);
            int view_Location = glGetUniformLocation(shaderId, ID_UNIFORM_VIEW);
            int proj_Location = glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION);

            glUniformMatrix4fv(model_Location, 1, GL_FALSE, (float*)transform.modelMatrix);
            glUniformMatrix4fv(view_Location, 1, GL_FALSE, (float*)camera.viewMatrix);
            glUniformMatrix4fv(proj_Location, 1, GL_FALSE, (float*)camera.projectionMatrix);

            // std::cout << "Floats: " << material.floats.size() << std::endl;
            // std::cout << "Vectors: " << material.vectors.size() << std::endl;
            // std::cout << "Textures: " << material.textures.size() << std::endl;

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
                glBindTexture(GL_TEXTURE_2D, texture.glHandle);
                glUniform1i(texParamName, textureNumber);
                textureNumber++;
                if (DBG_LVL < MAX_DBG) {
                    printf("Texture Handle %d, %d\n", texturePair.second.index, texturePair.second.generation);
                    printf("Texture GL handle %d\n", texture.glHandle);
                    printf("Texture Name: %s\n" ,texture.name);

                }
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



static int logTimes = 0;
static int MAX_LOG = 2;

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
                std::cout << "Set GLhandle for cubeMap: " << texturePair.first << std::endl;
            }

        }
        else {
            // error
        }
    }

    auto loc = glGetUniformLocation(shaderId, ID_SKYBOX_CUBEMAP);
    if (logTimes < MAX_LOG) {
        std::cout << " LOCATION " << loc << std::endl;
    }

    glUniform1i(loc, 0);

    glUniformMatrix4fv(glGetUniformLocation(shaderId, ID_UNIFORM_VIEW), 1, GL_FALSE, (float*)viewNoTranslation);
    glUniformMatrix4fv(glGetUniformLocation(shaderId, ID_UNIFORM_PROJECTION), 1, GL_FALSE, (float*)camera.projectionMatrix);

    glBindVertexArray(skybox.renderData.vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    glBindVertexArray(0);
    glDepthFunc(GL_LESS);
    glUseProgram(0);
}


void GL_InitDefaultMaterials(AssetManager& assets) {
    // Default 3d
    {
        auto& material = assets.materials.GetNewObjectAndHandle(assets.materialDefault3d);
        material.shaderName = "Default3D";
        material.SetVectorDefinition(ID_COLOR_TINT, {0.95f, 0.95f, 0.95f, 1.0});
        material.SetFloatDefinition(ID_SPECULAR_POWER, 45);
        material.SetTextureDefinition(ID_BASE_MAP, "");
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
}
