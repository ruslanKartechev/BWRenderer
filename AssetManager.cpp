#include "AssetManager.h"
#include <filesystem>
#include "Application.h"
#include <fstream>
#include <iostream>

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
std::string AssetManager::GetGlobalPathShaderModel(const char* resourcesPath)
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