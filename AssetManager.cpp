#include <filesystem>
#include <fstream>
#include <iostream>

#include "MyTypes.h"
#include "assimp/assimp/Importer.hpp"
#include "assimp/assimp/scene.h"
#include "assimp/assimp/postprocess.h"
#include "Application.h"
#include "AssetManager.h"

#include "RenderSubMesh.h"
#include "stb_image.h"


static const int MAX_STRCMP_ITERATIONS = 128;


std::string AssetManager::GetGlobalPath(const char* resourcesPath)
{
    std::filesystem::path fullPath = (Application::ResourcesPath);
    fullPath = fullPath / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}

std::string AssetManager::GetGlobalPathTextures(const char* resourcesPath)
{
    std::filesystem::path fullPath = (Application::ResourcesPath);
    fullPath = fullPath / "Textures" / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}

std::string AssetManager::GetGlobalPathShader(const char* resourcesPath)
{
    std::filesystem::path fullPath = (Application::ResourcesPath);
    fullPath = fullPath / "Shaders" / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}
std::string AssetManager::GetGlobalPathModel(const char* resourcesPath)
{
    std::filesystem::path fullPath = (Application::ResourcesPath);
    fullPath = fullPath / "Models" / resourcesPath;
    fullPath.make_preferred();
    return fullPath.string();
}


bool AssetManager::ReadStringContent(std::string &content, const char* path) {
    std::filesystem::path fullPath = (Application::ResourcesPath);
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
    out_mesh.stride = stride;
    out_mesh.vertexDataCount = ai_mesh.mNumVertices * stride;
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
    }
    out_mesh.indexCount = ai_mesh.mNumFaces * 3;
    out_mesh.indexData = new int[out_mesh.indexCount];
    int idxOffset = 0;
    for(unsigned int i = 0; i < ai_mesh.mNumFaces; i++) {
        const aiFace& face = ai_mesh.mFaces[i];
        for(unsigned int j = 0; j < face.mNumIndices; j++) {
            out_mesh.indexData[idxOffset++] = face.mIndices[j];
        }
    }
}


Handle ParseIntoRenderObject(aiNode* node, const aiScene& aiScene, GameScene& gameScene) {

    Handle objHandle;
    RenderObject& renderObject = gameScene.renderObjects.GetNewObjectAndHandle(objHandle);
    Transform& objTransform = gameScene.transforms.GetNewObjectAndHandle(renderObject.hTransform);
    Transform_Init(objTransform);

    renderObject.meshData.resize(node->mNumMeshes);
    renderObject.shadersAssigned = false;

    size_t renderMeshCount = 0;
    for (size_t i = 0; i < node->mNumMeshes; i++) {
        u32* nodeMeshes = node->mMeshes;
        u32 idx = nodeMeshes[i];
        if (aiScene.mMeshes[idx] == nullptr) {
            continue;
        }
        aiMesh& aiMesh = *aiScene.mMeshes[idx];

        Handle hMesh;
        Mesh& gameMesh = gameScene.meshes.GetNewObjectAndHandle(hMesh);
        ParseAssimpMesh(aiMesh, gameMesh);
        // printf("!! MESH HDL !! obj handle %d, %d\n", hMesh.index, hMesh.generation);
        // Mesh_Print(gameMesh);

        renderMeshCount++;
        if (renderObject.meshData.size() < renderMeshCount) {
            renderObject.meshData.resize(renderMeshCount);
        }
        RenderSubMesh& renderData = renderObject.meshData[renderMeshCount-1];
        renderData.hMesh = hMesh;
    }
    return objHandle;
}


void ParseSceneRecursive(aiNode* node, const aiScene& aiScene, GameScene& gameScene, Handle& rootHandle) {
    if(node->mNumMeshes > 0) {
        Handle newObjHandle = ParseIntoRenderObject(node, aiScene, gameScene);
        if (rootHandle.index == 0 && rootHandle.generation == 0) {
            rootHandle.generation = newObjHandle.generation;
            rootHandle.index = newObjHandle.index;
        }
    }
    for (size_t i = 0; i < node->mNumChildren; i++) {
        ParseSceneRecursive(node->mChildren[i], aiScene, gameScene, rootHandle);
    }
}


Handle AssetManager::LoadModel(const char* path, GameScene& gameScene, int num){
    auto filePath = GetGlobalPathModel(path);
    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(filePath, aiProcess_Triangulate | aiProcess_FlipUVs | aiProcess_GenSmoothNormals);
    if (scene == nullptr) {
        std::cerr << "FAILED TO LOAD SCENE" << std::endl;
        return {0,0};
    }
    if (scene->mRootNode == nullptr) {
        std::cerr << "The root node is null!" << std::endl;
        return {0,0};
    }
    Handle rootHandle = {0,0};
    ParseSceneRecursive(scene->mRootNode, *scene, gameScene, rootHandle);
    return rootHandle;
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

Shader& AssetManager::GetShader(Handle h) {
    // printf("[SR] Getting item ref %d, %d \n", h.index, h.generation);
    return shaders.GetItemRef(h);
}





// region Textures

Texture& AssetManager::GetNewTextureObject(Handle& outHandle) {
    auto& texture = textures.GetNewObjectAndHandle(outHandle);
    texture.isLoaded = false;
    return texture;
}


bool AssetManager::LoadTextureAtPath(Texture &texture, const char *relativePath, bool uploadToGPU) {
    int sizeX = 0;
    int sizeY = 0;
    int nrChannels = 0;
    texture.name = relativePath;
    texture.isLoaded = false;
    texture.name = relativePath;

    std::string gloalPath = GetGlobalPathTextures(relativePath);
    if (texture.pixels != nullptr) {
        free(texture.pixels);
    }
    texture.pixels = nullptr;
    texture.pixels = stbi_load(gloalPath.c_str(), &sizeX, &sizeY, &nrChannels, 4);
    if (texture.pixels == nullptr) {

        std::cerr << "[TextureLoad] Failed to real pixels! " << gloalPath  << std::endl;
        return false;
    }

    texture.width = sizeX;
    texture.height = sizeY;
    texture.channels = nrChannels;
    texture.isLoaded = true;
    if (uploadToGPU) {
        texture.UploadToGL(true);
    }

    std::cout << "---------- 1  " << texture.name << " GL HANDEL AfTER UPLOAD  " << texture.glHandle << std::endl;

    return true;
}

bool AssetManager::LoadTextureCubemap(Texture& texture, std::vector<std::string>& facePaths) {
    bool didLoad = true;

    if (facePaths.size() != 6) {
        std::cout << "Possible error. Cubemap sides count is not 6" << std::endl;
        texture.isLoaded = false;
        return false;
    }

    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureID);

    texture.glHandle = textureID;
    texture.isLoaded = true;

    for (size_t i = 0; i < facePaths.size(); i++) {
        i32 width, height, nrChannels;
        std::string loadPath = GetGlobalPathTextures(facePaths[i].c_str());
        std::cout << "Loading from: " << loadPath << std::endl;

        unsigned char* data = stbi_load(loadPath.c_str(), &width, &height, &nrChannels, 0);
        if (data) {
            // OpenGL enum go like: (Right, Left, Top, Bottom, Front, Back)
            GLenum format = (nrChannels == 4) ? GL_RGBA : GL_RGB;
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + i, 0, format, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
            stbi_image_free(data);
        }
        else {
            std::cout << "Failed to load cubemap texture at: " << facePaths[i] << std::endl;
            stbi_image_free(data);
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



void AssetManager::CreateDefaultWhiteTexture() {

    constexpr i32 dim = 4 ;
    constexpr i32 size = dim * dim;
    u8 whitePixels[4 * size];
    std::memset(whitePixels, 255, sizeof(whitePixels)); // Is this correct

    Texture& texture = GetNewTextureObject(defaultWhiteTexture);
    texture.pixels = whitePixels;
    texture.name = "white";
    texture.width = dim;
    texture.height = dim;

    texture.isLoaded = true;
    texture.UploadToGL(false);
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

        std::cout << "[Assets] texture name: " <<  obj.name << std::endl;
        if (std::strncmp(obj.name, name, MAX_STRCMP_ITERATIONS) == 0) {

            return Handle(idx, textures.GetGenerationFor(idx));
        }
        idx++;
    }
    std::cout << "[Assets] Failed to FIND matching texture!" << std::endl;
    return Handle(0,0);
}


Handle AssetManager::FindShaderByName(const char* name) {
    auto& vec = shaders.GetVector();
    i32 idx = 0;
    // std::cout << "shaders count : " << vec.size() << std::endl;
    for (auto& obj : vec) {
        // std::cout << "[search] " << obj.GetName() << " " << idx << std::endl;
        if (std::strncmp(obj.GetName().c_str(), name, MAX_STRCMP_ITERATIONS) == 0) {
            return Handle(idx, shaders.GetGenerationFor(idx));
        }
        idx++;
    }
    std::cerr << "[Assets] Failed to FIND matching shader!" << std::endl;
    return Handle(0,0);
}

// endregion
