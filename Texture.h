//
// Created by user on 5/30/2026.
//

#ifndef RENDERGL_TEXTURE_H
#define RENDERGL_TEXTURE_H


class Texture {
public:
    int width;
    int height;
    int channels;
    unsigned int glHandle;
    unsigned char* pixels;
    int pixelFormat;

    bool LoadTexture(const char* path);
    bool UploadToGL(bool freeResource = true);
};


#endif //RENDERGL_TEXTURE_H
