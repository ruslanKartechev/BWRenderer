#pragma once
#include "MyTypes.h"
#include <string>

class Texture {
public:
    i32 width = 0;
    i32 height = 0;
    i32 channels = 0;
    i32 pixelFormat = 0;
    u32 glHandle = 0;
    bool isLoaded = false;
    const char* name = "";
    unsigned char* pixels = nullptr;


    bool UploadToGL(bool freeResource = true);
};
