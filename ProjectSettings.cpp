#include "ProjectSettings.h"
#include <sstream>
#include <fstream>
#include <iostream>
#include "ProjectDefines.h"

std::string ProjectSettings::RootPath = {};
std::string ProjectSettings::ResourcesPath = {};

std::string GetSubstringNoSpaces(const std::string& str) {
    size_t first = str.find_first_not_of(" \t\r\n");
    if (first == std::string::npos)
        return "";
    size_t last = str.find_last_not_of(" \t\r\n");
    return str.substr(first, (last - first + 1));
}

void ParseVector(const std::string& value, vec4& out_vector) {
    std::string cleanValue = std::string(value);
    std::erase(cleanValue, '(');
    std::erase(cleanValue, ')');
    std::stringstream ss(cleanValue);
    std::string token;
    i32 i = 0;
    while (std::getline(ss, token, ',') && i < 4) {
        try {
            out_vector[i] = std::stof(GetSubstringNoSpaces(token));
        }
        catch (...) {
            out_vector[i] = 1.0f;
        }
        i++;
    }
}

bool ProjectSettings::LoadProjectSettings(const std::string& filePath, ProjectSettings& outSettings) {
    std::ifstream file(filePath);

#ifdef LOG_PROJECT_SETTINGS_READ
    std::cout << "[ProjectSettings] " << filePath << std::endl;
#endif
    if (!file.is_open()) {
        std::cerr << "[ProjectSettings] Failed to open settings file: " << filePath << std::endl;
        return false;
    }

    std::string line;
    while (std::getline(file, line)) {
        line = GetSubstringNoSpaces(line);
        // std::cout << "reading line " << line <<  std::endl;

        if (line.empty() || line[0] == '#' || line[0] == ';' || line[0] == '/') {
            continue;
        }
        size_t equalsPos = line.find('=');
        if (equalsPos == std::string::npos) {
            continue;
        }

        std::string key = GetSubstringNoSpaces(line.substr(0, equalsPos));
        std::string value = GetSubstringNoSpaces(line.substr(equalsPos + 1));
        bool isTrue = (value == "true" || value == "1");
#ifdef LOG_PROJECT_SETTINGS_READ
        std::cout << "[ProjectSettings] KEY "<<key<<"   VALUE: " << value << std::endl;
#endif
        try {
            // Bools
            if (key == "Gamma_Correction")
                outSettings.UseGammaCorrection = isTrue;
            else if (key == "Shadows")
                outSettings.RenderShadows = isTrue;

            else if (key == "Debug_UVs")
                outSettings.DevRenderUvs = isTrue;
            else if (key == "Debug_Depths")
                outSettings.DevRenderDepths = isTrue;
            else if (key == "Debug_Normals")
                outSettings.DevRenderNormals = isTrue;
            else if (key == "Debug_Colors")
                outSettings.DevRenderColors = isTrue;
            else if (key == "Debug_Light_View")
                outSettings.DebugVisualizeLightSources = isTrue;

            else if (key == "RenderSkyBox")
                outSettings.RenderSkyBox = isTrue;
            else if (key == "PostProcess")
                outSettings.PostProcess = isTrue;

            else if (key == "PostProcess_SSR")
                outSettings.PostProcess_SSR = isTrue;
            else if (key == "PostProcess_Bloom")
                outSettings.PostProcess_Bloom = isTrue;
            else if (key == "PostProcess_DOF")
                outSettings.PostProcess_DOF = isTrue;
            else if (key == "PostProcess_SSAO")
                outSettings.PostProcess_SSAO = isTrue;
            else if (key == "PostProcess_ToneMapping")
                outSettings.PostProcess_ToneMapping = isTrue;
            else if (key == "PostProcess_BW")
                outSettings.PostProcess_BW = isTrue;

            // Floats
            else if (key == "Ambient_Light_Brightness")
                outSettings.Ambient_Light_Brightness = std::stof(value);
            else if (key == "Direct_Light_Intensity")
                outSettings.Direct_Light_Intensity = std::stof(value);
            else if (key == "Camera_Move_Speed")
                outSettings.Camera_Move_Speed = std::stof(value);
            else if (key == "Camera_Rotation_Speed")
                outSettings.Camera_Rotation_Speed = std::stof(value);
            else if (key == "BloomExposure")
                outSettings.BloomExposure = std::stof(value);
            else if (key == "BloomThreshold")
                outSettings.BloomThreshold = std::stof(value);


            // Vectors
            else if (key == "Ambient_Light_Color")
                ParseVector(value, outSettings.Ambient_Light_Color);
            else if (key == "Direct_Light_Color")
                ParseVector(value, outSettings.Direct_Light_Color);

        }
        catch (const std::exception& e) {
            std::cerr << "Error parsing key '" << key << "': " << e.what() << std::endl;
            return false;
        }
    }
    return true;
}