#include "Material.h"



void Material::SetFloatDefinition(const std::string& name, float value) {
    floatsDefinitions.emplace_back(name, value);
}

void Material::SetVectorDefinition(const std::string& name, vector4 value) {
    vectorsDefinitions.emplace_back(name, value);

}

void Material::SetTextureDefinition(const std::string& name, std::string value) {
    texturesDefinitions.emplace_back(name, value);

}
