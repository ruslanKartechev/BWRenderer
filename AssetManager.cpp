#include <filesystem>
#include <fstream>
#include <iostream>

#include "MyTypes.h"
#include "assimp/assimp/Importer.hpp"
#include "assimp/assimp/scene.h"
#include "assimp/assimp/postprocess.h"
#include "Application.h"
#include "AssetManager.h"

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
    RenderObject& renderObject = gameScene.renderObjects.MakeNew(objHandle);
    Transform& objTransform = gameScene.transforms.MakeNew(renderObject.hTransform);
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
        Mesh& gameMesh = gameScene.meshes.MakeNew(hMesh);
        ParseAssimpMesh(aiMesh, gameMesh);
        printf("!! MESH HDL !! obj handle %d, %d\n", hMesh.index, hMesh.generation);
        Mesh_Print(gameMesh);

        renderMeshCount++;
        if (renderObject.meshData.size() < renderMeshCount) {
            renderObject.meshData.resize(renderMeshCount);
        }
        MeshRenderData& renderData = renderObject.meshData[renderMeshCount-1];
        renderData.hMesh = hMesh;
    }
    printf("Returninig obj handle %d, %d\n", objHandle.index, objHandle.generation);
    return objHandle;
}


void ParseSceneRecursive(aiNode* node, const aiScene& aiScene, GameScene& gameScene, Handle& rootHandle) {

    if(node->mNumMeshes > 0) {
        Handle tempHandle = ParseIntoRenderObject(node, aiScene, gameScene);
        if (rootHandle.index == 0 && rootHandle.generation == 0) {

            rootHandle.generation = tempHandle.generation;
            rootHandle.index = tempHandle.index;
        }
    }

    for (size_t i = 0; i < node->mNumChildren; i++) {
        ParseSceneRecursive(node->mChildren[i], aiScene, gameScene, rootHandle);
    }
}


Handle AssetManager::LoadModel(const char* path, GameScene& gameScene)
{
    auto filePath = GetGlobalPathModel(path);
    std::cout << "Full model path: " << filePath << std::endl;

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

    printf("FINAL Returninig obj handle %d, %d\n", rootHandle.index, rootHandle.generation);
    return rootHandle;
}