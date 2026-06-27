#ifndef RENDERGL_ASSETMANAGER_H
#define RENDERGL_ASSETMANAGER_H
#include <string>


class AssetManager {
public:

    static std::string GetGlobalPath(const char* resourcesPath);

    static std::string GetGlobalPathTextures(const char* resourcesPath);

    static std::string GetGlobalPathShader(const char* resourcesPath);

    static std::string GetGlobalPathShaderModel(const char* resourcesPath);

    static bool ReadStringContent(std::string &content, const char* path);
};


#endif //RENDERGL_ASSETMANAGER_H
