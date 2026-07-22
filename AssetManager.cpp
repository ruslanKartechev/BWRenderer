#include <filesystem>
#include <fstream>
#include <iostream>

#include "MyTypes.h"
#include "assimp/assimp/Importer.hpp"
#include "assimp/assimp/postprocess.h"
#include "AssetManager.h"

#include "ProjectSettings.h"
#include "RenderSubMesh.h"
#include "stb_image.h"
#include "Uniforms.h"


static const int MAX_STRCMP_ITERATIONS = 128;


std::string AssetManager::GetProjectSettingsPath() {
    return GetGlobalPath("Settings.ini");
}

std::string AssetManager::GetGlobalPath(const char* resourcesPath)
{
    std::filesystem::path fullPath = (ProjectSettings::ResourcesPath);
    fullPath = fullPath / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}

std::string AssetManager::GetGlobalPathTextures(const char* resourcesPath)
{
    std::filesystem::path fullPath = (ProjectSettings::ResourcesPath);
    fullPath = fullPath / "Textures" / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}

std::string AssetManager::GetGlobalPathShader(const char* resourcesPath)
{
    std::filesystem::path fullPath = (ProjectSettings::ResourcesPath);
    fullPath = fullPath / "Shaders" / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}
std::string AssetManager::GetGlobalPathModel(const char* resourcesPath)
{
    std::filesystem::path fullPath = (ProjectSettings::ResourcesPath);
    fullPath = fullPath / "Models" / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}


bool AssetManager::ReadStringContent(std::string &content, const char* path) {
    std::filesystem::path fullPath = (ProjectSettings::ResourcesPath);
    fullPath = fullPath / path;
    fullPath.make_preferred(); // convert slash to backslash (or vice versa)

    std::ifstream file(fullPath.string(), std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        fprintf(stderr, "Failed to load content at path: %s\n", fullPath.string().c_str());
        return false;
    }
    size_t fileSize = file.tellg();
    content.resize(fileSize);
    file.seekg(0);
    file.read(content.data(), fileSize);
    file.close();
    return true;
}



void ParseAssimpMesh(const aiMesh& ai_mesh, Mesh& out_mesh) {
    int stride = 3;
    out_mesh.startIndexVertex = 0;
    out_mesh.startIndexUV = -1;
    out_mesh.startIndexNormals = -1;
    out_mesh.startIndexColor = -1;
    out_mesh.startIndexTangent = -1;
    if(ai_mesh.HasTextureCoords(0)) {
        out_mesh.startIndexUV = stride;
        stride += 2;
    }
    if(ai_mesh.HasNormals()) {
        out_mesh.startIndexNormals = stride;
        stride += 3;
    }
    if(ai_mesh.HasVertexColors(0)) {
        out_mesh.startIndexColor = stride;
        stride += 4;
    }
    if(ai_mesh.HasTangentsAndBitangents()) {
        out_mesh.startIndexTangent = stride;
        stride += 3;
    }
    out_mesh.stride = stride;
    out_mesh.vertexDataCount = static_cast<i32>(ai_mesh.mNumVertices) * stride;
    out_mesh.vertexData = new float[out_mesh.vertexDataCount];
    int offset = 0;
    for(unsigned int i = 0; i < ai_mesh.mNumVertices; i++) {
        out_mesh.vertexData[offset++] = ai_mesh.mVertices[i].x;
        out_mesh.vertexData[offset++] = ai_mesh.mVertices[i].y;
        out_mesh.vertexData[offset++] = ai_mesh.mVertices[i].z;
        if(out_mesh.startIndexUV != -1) {
            out_mesh.vertexData[offset++] = ai_mesh.mTextureCoords[0][i].x;
            out_mesh.vertexData[offset++] = ai_mesh.mTextureCoords[0][i].y;
        }
        if(out_mesh.startIndexNormals != -1) {
            out_mesh.vertexData[offset++] = ai_mesh.mNormals[i].x;
            out_mesh.vertexData[offset++] = ai_mesh.mNormals[i].y;
            out_mesh.vertexData[offset++] = ai_mesh.mNormals[i].z;
        }
        if(out_mesh.startIndexColor != -1) {
            out_mesh.vertexData[offset++] = ai_mesh.mColors[0][i].r;
            out_mesh.vertexData[offset++] = ai_mesh.mColors[0][i].g;
            out_mesh.vertexData[offset++] = ai_mesh.mColors[0][i].b;
            out_mesh.vertexData[offset++] = ai_mesh.mColors[0][i].a;
        }
        if (out_mesh.startIndexTangent != -1) {
            out_mesh.vertexData[offset++] = ai_mesh.mTangents[i].x;
            out_mesh.vertexData[offset++] = ai_mesh.mTangents[i].y;
            out_mesh.vertexData[offset++] = ai_mesh.mTangents[i].z;
        }
    }
    out_mesh.indexCount = static_cast<i32>(ai_mesh.mNumFaces * 3);
    out_mesh.indexData = new int[out_mesh.indexCount];
    int idxOffset = 0;

    for(u32 i = 0; i < ai_mesh.mNumFaces; i++) {

        const aiFace& face = ai_mesh.mFaces[i];
        for(u32 j = 0; j < face.mNumIndices; j++) {
            out_mesh.indexData[idxOffset++] = face.mIndices[j];
        }
    }
}


Handle AssetManager::ParseIntoRenderObject(const aiNode* node, const aiScene& aiScene, GameScene& gameScene) {
    Handle objHandle {0,0};
    RenderObject& renderObject = gameScene.worldObjectsPool.GetNewObjectAndHandle(objHandle);
    Transform& objTransform = gameScene.transforms.GetNewObjectAndHandle(renderObject.hTransform);
    Transform_Init(objTransform);

    renderObject.subMeshses.resize(node->mNumMeshes);
    renderObject.name = std::string(node->mName.C_Str());

    size_t renderMeshCount = 0;
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        u32* nodeMeshes = node->mMeshes;
        u32 idx = nodeMeshes[i];
        if (aiScene.mMeshes[idx] == nullptr) {
            continue;
        }
        aiMesh& aiMesh = *aiScene.mMeshes[idx];

        Handle hMesh = {0,0};
        Mesh& gameMesh = meshes.GetNewObjectAndHandle(hMesh);
        gameMesh.name = std::string(aiMesh.mName.C_Str());
        ParseAssimpMesh(aiMesh, gameMesh);

        renderMeshCount++;
        if (renderObject.subMeshses.size() < renderMeshCount) {
            renderObject.subMeshses.resize(renderMeshCount);
        }
        RenderSubMesh& renderData = renderObject.subMeshses[renderMeshCount-1];
        renderData.hMesh = hMesh;
    }
    return objHandle;
}

bool AssetManager::ParseSceneRecursive(const aiNode* node, const aiScene& aiScene, GameScene& gameScene, std::vector<Handle>& newHandles) {
    if(node->mNumMeshes > 0) {
        Handle newObjHandle = ParseIntoRenderObject(node, aiScene, gameScene);
        newHandles.push_back(newObjHandle);
    }
    for (size_t i = 0; i < node->mNumChildren; i++) {
        ParseSceneRecursive(node->mChildren[i], aiScene, gameScene, newHandles);
    }
    return true;
}


bool AssetManager::ParseIntoObjectDefinition(const aiNode* node, const aiScene& aiScene, ObjectDefinition& definition) {

    size_t renderMeshCount = 0;
    definition.name = std::string(node->mName.C_Str());
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        u32* nodeMeshes = node->mMeshes;
        u32 idx = nodeMeshes[i];
        if (aiScene.mMeshes[idx] == nullptr) {
            continue;
        }
        aiMesh& aiMesh = *aiScene.mMeshes[idx];

        Handle hMesh = {0,0};
        Mesh& gameMesh = meshes.GetNewObjectAndHandle(hMesh);
        gameMesh.name = std::string(aiMesh.mName.C_Str());
        ParseAssimpMesh(aiMesh, gameMesh);
        definition.Meshes.push_back(hMesh);
        renderMeshCount++;
    }
    return true;
}

/// Parses the first non-empty mesh only
bool AssetManager::ParseDefinitionsRecursive(aiNode* node, const aiScene& aiScene,
                    std::vector<ObjectDefinition>& definitions) {

    if(node->mNumMeshes > 0) {
        definitions.emplace_back();
        auto& newObj = definitions[definitions.size()-1];
        ParseIntoObjectDefinition(node, aiScene, newObj);
    }
    for (size_t i = 0; i < node->mNumChildren; i++) {
        ParseDefinitionsRecursive(node->mChildren[i], aiScene, definitions);
    }
    return true;
}


int AssetManager::LoadDefinitions(const char* path, std::vector<ObjectDefinition>& definitions){

    auto filePath = GetGlobalPathModel(path);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate
        | aiProcess_FlipUVs
        | aiProcess_GenSmoothNormals
        | aiProcess_CalcTangentSpace);

    if (scene == nullptr) {
        std::cerr << "FAILED TO LOAD SCENE" << std::endl;
        return 1;
    }
    if (scene->mRootNode == nullptr) {
        std::cerr << "The root node is null!" << std::endl;
        return 2;
    }
    ParseDefinitionsRecursive(scene->mRootNode, *scene, definitions);
    return 0;
}




int AssetManager::LoadModelsFromFbx(const char *path, GameScene &gameScene, std::vector<Handle> &objectsHandles) {
    auto filePath = GetGlobalPathModel(path);
    Assimp::Importer importer;
    const aiScene *scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace);
    if (scene == nullptr) {
        std::cerr << "FAILED TO LOAD SCENE" << std::endl;
        return 1;
    }
    if (scene->mRootNode == nullptr) {
        std::cerr << "The root node is null!" << std::endl;
        return 2;
    }
    ParseSceneRecursive(scene->mRootNode, *scene, gameScene, objectsHandles);
    return 0;
}



Shader& AssetManager::GetDefault3D() {
    // printf("[SR]  Shader RETURNING %d, %d\n", default3D.index, default3D.generation);
    return shaders.GetItemRef(shaderDefault3D);
}

Shader& AssetManager::GetDefault2D() {
    // printf("[SR] Shader RETURNING %d, %d\n", default2D.index, default2D.generation);
    return shaders.GetItemRef(shaderDefault2D);
}
Shader& AssetManager::GetSkyboxDefault() {
    return shaders.GetItemRef(shaderDefaultSkybox);
}
Shader& AssetManager::GetFallback() {
    return shaders.GetItemRef(shaderFallback);
}
Shader& AssetManager::GetScreenRenderTextureShader() {
    return shaders.GetItemRef(shaderScreenRenderTexture);
}
Shader& AssetManager::GetShader(Handle h) {
    return shaders.GetItemRef(h);
}
Shader& AssetManager::GetShaderSSR() {
    return shaders.GetItemRef(shaderSSR);
}


// region Textures

Texture& AssetManager::GetNewTextureObject(Handle& outHandle) {
    auto& texture = textures.GetNewObjectAndHandle(outHandle);
    texture.isLoaded = false;
    return texture;
}


bool AssetManager::LoadTexturesForMaterials(Material& material) {
    bool loaded = true;
    for (auto& definitionPair : material.texturesDefinitions) {
        Handle h = {};
        std::string& path = definitionPair.second.first;
        std::cout << "[Assets.LOADING_Tex] " << path << " , " << std::endl;
        Texture& texture = GetNewTextureObject(h);
        bool didLoad = LoadTextureAtPath(texture, path.c_str(), true);
        loaded &= didLoad;
    }
    return loaded;
}


bool AssetManager::LoadTextureAtPath(Texture &texture, const char *relativePath, bool uploadToGPU) {
    int sizeX = 0;
    int sizeY = 0;
    int nrChannels = 0;
    texture.name = relativePath;
    texture.isLoaded = false;
    texture.name = relativePath;
    texture.FreeData();

    std::string assetPath = GetGlobalPathTextures(relativePath);
    u8* ptr = stbi_load(assetPath.c_str(), &sizeX, &sizeY, &nrChannels, 4);
    if (ptr == nullptr) {
        std::cerr << "[Assets] Failed to read pixels! " << assetPath  << std::endl;
        return false;
    }

    texture.width = sizeX;
    texture.height = sizeY;
    texture.channels = nrChannels;
    texture.SetPixelFormat(Texture::TEX_FORMAT_sRGB32);
    texture.SetBytePixelsPtr(ptr);
    texture.isLoaded = true;
    if (uploadToGPU) {
        texture.UploadToGL(true);
    }
    return true;
}


bool AssetManager::LoadTextureCubeMap6Face(Texture& texture, std::vector<std::string>& facePaths) {
    bool didLoad = true;

    if (facePaths.size() != 6) {
        std::cerr << "[Assets] Possible error. Cubemap sides count is not 6" << std::endl;
        texture.isLoaded = false;
        return false;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    texture.glHandle = textureID;
    texture.isLoaded = true;
    texture.SetPixelFormat(Texture::TEX_FORMAT_SkyBox);

    stbi_set_flip_vertically_on_load(false);

    for (size_t i = 0; i < facePaths.size(); i++) {
        i32 width, height, nrChannels;
        std::string loadPath = GetGlobalPathTextures(facePaths[i].c_str());
        // std::cout << "[Assets] Loading part of image from: " << loadPath << std::endl;

        unsigned char* pixelData = stbi_load(loadPath.c_str(), &width, &height, &nrChannels, 0);
        if (pixelData != nullptr) {
            // OpenGL enum go like: (Right, Left, Top, Bottom, Front, Back)
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, pixelData);
            stbi_image_free(pixelData);
        }
        else {
            std::cout << "Failed to load cubemap texture at: " << facePaths[i] << std::endl;
            didLoad = false;
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);

    return didLoad;
}



void AssetManager::CreateDefaultNormalMap() {
    constexpr i32 dimensions = 4;
    Texture& texture = GetNewTextureObject(defaultNormal);
    texture.GenerateDefaultNormalTexture(dimensions);
    texture.name = TEX_DEFAULT_NORMAL;
}

void AssetManager::CreateDefaultWhiteTexture() {
    constexpr i32 dim = 4 ;
    Texture& texture = GetNewTextureObject(defaultWhiteTexture);
    texture.GenerateDefaultWhiteTexture(dim);
    texture.name = TEX_DEFAULT_WHITE;

}
//endregion



// region Find By Name
Handle AssetManager::FindTextureByName(const char* name) {
    auto& vec = textures.GetVector();
    i32 idx = 0;
    for (auto& obj : vec) {

        if (obj.name == nullptr) {
            continue;
        }
        // std::cout << "[AssetsSearch] texture name: " <<  obj.name << std::endl;
        if (std::strncmp(obj.name, name, MAX_STRCMP_ITERATIONS) == 0) {

            return Handle(idx, textures.GetGenerationFor(idx));
        }
        idx++;
    }
    std::cerr << "[Assets] 404 Shader by texture: " << name << std::endl;
    return Handle(0,0);
}


Handle AssetManager::FindShaderByName(const char* name) {
    auto& vec = shaders.GetVector();
    i32 idx = 0;
    for (auto& obj : vec) {
        if (std::strncmp(obj.GetName().c_str(), name, MAX_STRCMP_ITERATIONS) == 0) {
            return Handle(idx, shaders.GetGenerationFor(idx));
        }
        idx++;
    }
    std::cerr << "[Assets] 404 Shader by name: " << name << std::endl;
    return Handle(0,0);
}


Handle AssetManager::FindMeshByName(const char* name)
{
    auto& vec = meshes.GetVector();
    i32 idx = 0;
    for (auto& obj : vec) {
        if (std::strncmp(obj.name.c_str(), name, MAX_STRCMP_ITERATIONS) == 0) {
            return Handle(idx, meshes.GetGenerationFor(idx));
        }
        idx++;
    }
    std::cerr << "[Assets] 404 Mesh by name: " << name << std::endl;
    return Handle(0,0);
}
// endregion
