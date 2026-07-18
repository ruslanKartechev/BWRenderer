#include "Material.h"
#include <algorithm>
#include <iostream>

void Material::SetFloatDefinition(const std::string& name, float value) {
    for (auto& pair : floatsDefinitions) {
        if (name.compare(pair.first) == 0) {
            pair.second = value;
            return;
        }
    }
    floatsDefinitions.emplace_back(name, value);
}

void Material::SetVectorDefinition(const std::string& name, vector4 value) {
    for (auto& pair : vectorsDefinitions) {
        if (name.compare(pair.first) == 0) {
            pair.second = value;
            return;
        }
    }
    vectorsDefinitions.emplace_back(name, value);

}
void Material::SetTextureDefinition(const std::string& name, std::string value, i32 binding) {
    for (auto& pair : texturesDefinitions) {
        if (name.compare(pair.first) == 0) {
            pair.second = std::pair(value, binding);
            return;
        }
    }
    texturesDefinitions.emplace_back(name, NameBindingPair{value, binding});
}
