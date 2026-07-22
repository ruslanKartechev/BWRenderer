#pragma once
#include "MyTypes.h"
#include <string>

class Texture {
public:
    static constexpr i32 TEX_FORMAT_sRGB32 = 0;
    static constexpr i32 TEX_FORMAT_R32 = 1;
    static constexpr i32 TEX_FORMAT_SkyBox = 2;

    i32 width = 0;
    i32 height = 0;
    i32 channels = 0;
    /// Data format stored in texture
    /// 0 - regular sRGB 32
    /// 1 - R32F single float data
    i32 pixelFormat = 0;
    u32 glHandle = 0;

    bool isLoaded = false;
    const char* name = "";


    bool UploadToGL(bool freeResource = true);

    /// Saves pixel data ptr as its own pixel data
    /// If the texture already had a pixel array, if will be FREEd before reassignment!
    void SetFloatDataPtr(f32* ptr);

    f32* GetFloatDataPtr();

    /// Saves pixel data ptr as its own pixel data
    /// If the texture already had a pixel array, if will be FREEd before reassignment!
    void SetBytePixelsPtr(u8* ptr);

    u8* GetBytePixelsPtr();

    /// Frees pixel data memory allocated on the cpu
    void FreeData();

    /// Generates [size, size] white RGBA white texture and uploads it gpu
    /// Does NOT store pixel data in cpu ram
    void GenerateDefaultWhiteTexture(i32 size = 4);

    /// Generates [size, size] default up-facing normal and uploads it gpu
    /// Does NOT store pixel data in cpu ram
    void GenerateDefaultNormalTexture(i32 size = 4);

    void SetPixelFormat(i32 format);
    void SetSize(i32 width, i32 height);
    void SetChannelCount(i32 channelCount);


private:
    u8* pixelsBytePtr = nullptr;
    f32* pixelsFloatPtr = nullptr;
};
