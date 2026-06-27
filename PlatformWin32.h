#ifndef RENDERGL_GL_LOADER_H
#define RENDERGL_GL_LOADER_H

#define WIN32_LEAN_AND_MEAN

#include <windows.h>
#include <cstdint>
#define GL_VER_MAJOR 3
#define GL_VER_MINOR 3

typedef int (*ResizeCallback)(uint32_t width, uint32_t height);
typedef int (*MoveCallback)(uint32_t width, uint32_t height);
typedef int (*EventCallback)();


typedef struct WindowParams{
    const char* name;
    uint32_t width;
    uint32_t height;

    uint32_t posX;
    uint32_t posY;

    HWND hwnd;
    HINSTANCE hInst;
    HDC dc;
    HGLRC glRc;
    bool close;

    ResizeCallback callbackResize;
    MoveCallback callbackMove;
    EventCallback callbackClose;
    EventCallback callbackDestroy;
    EventCallback callbackFocus;
    EventCallback callbackOutOfFocus;
    EventCallback callbackMinimized;

}WindowParams;


bool CreateFirstWindowAndInitGL(WindowParams* params);

void Win32WindowUpdate(const WindowParams& params);


#endif //RENDERGL_GL_LOADER_H
