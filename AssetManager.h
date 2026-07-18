#pragma once
#include <string>
#include "Material.h"
#include "Texture.h"
#include "Handle.h"
#include "GameScene.h"
#include "assimp/assimp/scene.h"
#include "Shader.h"

class AssetManager {
public:

    // Default Shaders
    Handle shaderDefault3D;
    Handle shaderDefault3DLight;

    Handle shaderDefault2D;
    Handle shaderDefaultSkybox;
    Handle shaderFallback;
    Handle shaderLightDebug;
    Handle shaderScreenRenderTexture;
    // Dev
    Handle shaderDepthOnly;
    Handle shaderNormalsOnly;
    Handle shaderColorOnly;

    // Post Process Shaders
    Handle shaderSSR;
    Handle shaderBloomDownsampleFirst;
    Handle shaderBloomDownsample;
    Handle shaderBloomUpSample;
    Handle shaderBloomComposite;

    Handle shaderBlur;
    Handle shaderColorCorrection;

    // Default Materials
    Handle materialDebug;
    Handle materialSkybox;
    Handle materialDefault3d;
    Handle materialDefault2d;

    // Default meshes
    Handle meshCube;
    Handle meshSphere;
    Handle meshCapsule;
    Handle meshPlane;
    Handle meshQuad;
    Handle meshPyramid;
    Handle meshDonut;
    Handle meshCutCone;

    // Per-type data storage
    SlotMap<Shader> shaders = {};
    SlotMap<Material> materials = {};
    SlotMap<Texture> textures = {};
    SlotMap<Mesh> meshes = {};

    // Textures
    Handle defaultWhiteTexture = {};
    Handle defaultNormal = {};


    static std::string GetGlobalPath(const char* resourcesPath);
    static std::string GetProjectSettingsPath();

    static std::string GetGlobalPathTextures(const char* resourcesPath);

    static std::string GetGlobalPathShader(const char* resourcesPath);

    static std::string GetGlobalPathModel(const char* resourcesPath);

    static bool ReadStringContent(std::string& content, const char* path);

    bool LoadTexturesForMaterials(Material& material);

    int LoadModelsFromFbx(const char* path, GameScene& gameScene, std::vector<Handle>& objectsHandles);

    int LoadDefinitions(const char* path, std::vector<ObjectDefinition>& newMeshHandles);

    /**
     * @param texture reference to a pre-allocated texture Object
     * @param relativePath Path to file relative in Resources Folder
     * @param uploadToGPU Upload immediately to GPU memory (GL) and free CPU resources ?
     * @return true if successfully read the file, false otherswise
     */
    static bool LoadTextureAtPath(Texture& texture, const char* relativePath, bool uploadToGPU);

    static bool LoadTextureCubemap(Texture& texture, std::vector<std::string>& facePaths);

    Texture& GetNewTextureObject(Handle& outHandle);

    void CreateDefaultWhiteTexture();

    void CreateDefaultNormalMap();

    Shader& GetDefault3D();
    Shader& GetDefault2D();
    Shader& GetSkyboxDefault();
    Shader& GetFallback();
    Shader& GetScreenRenderTextureShader();
    Shader& GetShader(Handle h);
    Shader& GetShaderSSR();
    Shader& GetShaderBloom();

    Handle FindTextureByName(const char* name);

    Handle FindShaderByName(const char* name);

    Handle FindMeshByName(const char* name);

private:
    Handle ParseIntoRenderObject(const aiNode* node, const aiScene& aiScene, GameScene& gameScene);
    bool ParseSceneRecursive(const aiNode* node, const aiScene& aiScene, GameScene& gameScene, std::vector<Handle>& newHandles);

    bool ParseIntoObjectDefinition(const aiNode* node, const aiScene& aiScene, ObjectDefinition& definition);
    bool ParseDefinitionsRecursive(aiNode* node, const aiScene& aiScene, std::vector<ObjectDefinition>& definitions);
};


