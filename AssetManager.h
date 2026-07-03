#pragma once
#include <string>
#include "GameScene.h"

class AssetManager {
public:

    static std::string GetGlobalPath(const char* resourcesPath);

    static std::string GetGlobalPathTextures(const char* resourcesPath);

    static std::string GetGlobalPathShader(const char* resourcesPath);

    static std::string GetGlobalPathModel(const char* resourcesPath);

    static bool ReadStringContent(std::string& content, const char* path);

    static Handle LoadModel(const char* path, GameScene& gameScene);
};


