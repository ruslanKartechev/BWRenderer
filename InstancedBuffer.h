#pragma once
#include <vector>
#include <iostream>

#include <string>
#include "Material.h"
#include "RenderObject.h"
#include "cglm/cglm.h"


struct InstanceEntry {
    mat4 modelMatrix;
};

class InstancedBuffer {
public:
    std::string name = {};
    std::vector<InstanceEntry> entries = {};
    u32 arrayObject = {0};
    RenderSubMesh renderData = {};
    InstanceEntry& AddNewOne();

};