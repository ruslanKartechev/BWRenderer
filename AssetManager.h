#pragma once
#include <string>
#include "Shader.h"
#include "Material.h"
#include "Texture.h"
#include "Handle.h"
#include "GameScene.h"

class AssetManager {
public:

    // Default Shaders
    Handle shaderDefault3D;
    Handle shaderDefault2D;
    Handle shaderDefaultSkybox;
    Handle shaderFallback;
    Handle shaderLightDebug;

    // Default Materials
    Handle materialDebug;
    Handle materialSkybox;
    Handle materialDefault3d;
    Handle materialDefault2d;

    // Per-type data storage
    SlotMap<Shader> shaders = {};
    SlotMap<Material> materials = {};
    SlotMap<Texture> textures = {};

    // Textures
    Handle defaultWhiteTexture = {};


    static std::string GetGlobalPath(const char* resourcesPath);

    static std::string GetGlobalPathTextures(const char* resourcesPath);

    static std::string GetGlobalPathShader(const char* resourcesPath);

    static std::string GetGlobalPathModel(const char* resourcesPath);

    static bool ReadStringContent(std::string& content, const char* path);

    static Handle LoadModel(const char* path, GameScene& gameScene, int num);

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


    Shader& GetDefault3D();
    Shader& GetDefault2D();
    Shader& GetSkyboxDefault();
    Shader& GetFallback();
    Shader& GetShader(Handle h);

    Handle FindTextureByName(const char* name);
    Handle FindShaderByName(const char* name);

};


