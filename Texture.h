#pragma once
#include "MyTypes.h"
#include <string>

class Texture {
public:
    i32 width;
    i32 height;
    i32 channels;
    i32 pixelFormat;
    u32 glHandle;
    bool isLoaded;
    const char* name;
    unsigned char* pixels;


    bool UploadToGL(bool freeResource = true);
};
