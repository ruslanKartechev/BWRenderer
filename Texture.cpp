#include "Texture.h"
#include "stb_image.h"
#include "glad/glad.h"


void Texture::FreeData()
{
    if (pixelsBytePtr != nullptr) {
        stbi_image_free(pixelsBytePtr);
    }
    if (pixelsFloatPtr != nullptr) {
        stbi_image_free(pixelsFloatPtr);
    }
}

u8* Texture::GetBytePixelsPtr() {
    return pixelsBytePtr;
}

f32* Texture::GetFloatDataPtr() {
    return pixelsFloatPtr;
}

void Texture::SetBytePixelsPtr(u8* ptr) {
    pixelsBytePtr = ptr;
    isLoaded = ptr != nullptr;
}

void Texture::SetFloatDataPtr(f32* ptr) {
    pixelsFloatPtr = ptr;
    isLoaded = ptr != nullptr;
}

void Texture::SetPixelFormat(i32 format) {
    pixelFormat = format;
}

void Texture::SetChannelCount(i32 count) {
    channels = count;
}


void Texture::SetSize(i32 width, i32 height) {
    this->width = width;
    this->height = height;
}


bool Texture::UploadToGL(bool freeResource) {

    glGenTextures(1, &glHandle);
    glBindTexture(GL_TEXTURE_2D, glHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    switch (pixelFormat) {
        case Texture::TEX_FORMAT_R32:
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, width, height, 0, GL_RED, GL_FLOAT, pixelsFloatPtr);
            break;
        case Texture::TEX_FORMAT_sRGB32:
            glPixelStorei(GL_UNPACK_ALIGNMENT, 4);
            glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixelsBytePtr);
            glGenerateMipmap(GL_TEXTURE_2D);
            break;
    }

    if (freeResource) {
        FreeData();
    }
    glBindTexture(GL_TEXTURE_2D, 0);

    return true;
}


void Texture::GenerateDefaultWhiteTexture(i32 dim) {
    i32 size = dim * dim;
    imax dataSize = sizeof(float) * size;
    pixelsBytePtr = new u8[dataSize];
    std::memset(pixelsBytePtr, 255, dataSize);
    width = dim;
    height = dim;
    isLoaded = true;
    UploadToGL(false);

    delete[] pixelsBytePtr;
    pixelsBytePtr = nullptr;
}

void Texture::GenerateDefaultNormalTexture(i32 dim) {
    i32 size = dim * dim;
    imax dataSize = sizeof(float) * size;
    pixelsBytePtr = new u8[dataSize];
    std::memset(pixelsBytePtr, 255, dataSize);
    for (size_t i = 0; i < size; i++) {
        pixelsBytePtr[i * 4 + 0] = 128; // x
        pixelsBytePtr[i * 4 + 1] = 128; // y
        pixelsBytePtr[i * 4 + 2] = 255; // z
        pixelsBytePtr[i * 4 + 3] = 255; // a
    }
    width = dim;
    height = dim;

    isLoaded = true;
    UploadToGL(false);

    delete[] pixelsBytePtr;
    pixelsBytePtr = nullptr;
}