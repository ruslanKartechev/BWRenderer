//
// Created by user on 5/30/2026.
//

#include "Texture.h"
#include <string>
#include "AssetManager.h"
#include "stb_image.h"
#include "glad/glad.h"



bool Texture::UploadToGL(bool freeResource) {

    glGenTextures(1, &this->glHandle);
    glBindTexture(GL_TEXTURE_2D, this->glHandle);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, this->width, this->height, 0, GL_RGBA, GL_UNSIGNED_BYTE, this->pixels);
    glGenerateMipmap(GL_TEXTURE_2D);

    if (freeResource) {
        stbi_image_free(this->pixels);
    }
    return true;
}

bool Texture::LoadTexture(const char* path) {
    int sizeX;
    int sizeY;
    int nrChannels;
    std::string gloalPath = AssetManager::GetGlobalPathTextures(path);
    fprintf(stdout, "LOADING PATH %s \n", gloalPath.c_str());
    this->pixels = nullptr;
    this->pixels = stbi_load(gloalPath.c_str(), &sizeX, &sizeY, &nrChannels, 4);
    if (pixels == nullptr) {
        fprintf(stderr, "FAILED TO LOAD TEXTURE\n");
        return false;
    }
    fprintf(stdout, "Loaded texture success\n");
    fprintf(stdout, "Texture Size %d by %d\n", sizeX, sizeY);
    this->width = sizeX;
    this->height = sizeY;
    this->channels = nrChannels;
    return true;
}
