#pragma once
#include <string>
#include "Material.h"
#include "Texture.h"
#include "Handle.h"
#include "GameScene.h"
#include "assimp/assimp/scene.h"
#include "Shader.h"
#include "Mesh.h"

class AssetManager {
public:

    // Default Shaders
    ShaderHandle shaderDefault3D;
    ShaderHandle shaderDefault3DLight;
    ShaderHandle shaderDefault2D;
    ShaderHandle shaderDefaultSkybox;
    ShaderHandle shaderFallback;
    ShaderHandle shaderLightDebug;
    ShaderHandle shaderScreenRenderTexture;
    ShaderHandle shaderTerrain;
    ShaderHandle uiQuad;

    // Dev
    ShaderHandle shaderDepthOnly;
    ShaderHandle shaderNormalsOnly;
    ShaderHandle shaderColorOnly;
    // Post Process Shaders
    ShaderHandle shaderSSR;
    ShaderHandle shaderBloomDownsampleFirst;
    ShaderHandle shaderBloomDownsample;
    ShaderHandle shaderBloomUpSample;
    ShaderHandle shaderBloomComposite;
    ShaderHandle shaderBlur;
    ShaderHandle shaderColorCorrection;
    

    // Default Materials
    MaterialHandle materialDebug;
    MaterialHandle materialSkybox;
    MaterialHandle materialDefault3d;
    MaterialHandle materialDefault2d;


    // Default meshes
    MeshHandle meshCube;
    MeshHandle meshSphere;
    MeshHandle meshCapsule;
    MeshHandle meshPlane;
    MeshHandle meshQuad;
    MeshHandle meshPyramid;
    MeshHandle meshDonut;
    MeshHandle meshCutCone;

    // Default Textures
    TextureHandle defaultWhiteTexture = {};
    TextureHandle defaultNormal = {};

    // Per-type data storage
    SlotMap<Shader> shaders = {};
    SlotMap<Material> materials = {};
    SlotMap<Texture> textures = {};
    SlotMap<Mesh> meshes = {};

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

    static bool LoadTextureCubeMap6Face(Texture& texture, std::vector<std::string>& facePaths);

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

    TextureHandle FindTextureByName(const char* name);

    ShaderHandle FindShaderByName(const char* name);

    MeshHandle FindMeshByName(const char* name);


private:
    Handle ParseIntoRenderObject(const aiNode* node, const aiScene& aiScene, GameScene& gameScene);
    bool ParseSceneRecursive(const aiNode* node, const aiScene& aiScene, GameScene& gameScene, std::vector<Handle>& newHandles);

    bool ParseIntoObjectDefinition(const aiNode* node, const aiScene& aiScene, ObjectDefinition& definition);
    bool ParseDefinitionsRecursive(aiNode* node, const aiScene& aiScene, std::vector<ObjectDefinition>& definitions);
};


