#include "glad/glad.h"
#include "PlatformWin32.h"
#include "InputSystem.h"
#include <iostream>

#define WGL_CONTEXT_MAJOR_VERSION_ARB     0x2091
#define WGL_CONTEXT_MINOR_VERSION_ARB     0x2092
#define WGL_CONTEXT_FLAGS_ARB             0x2094
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001
#define WGL_CONTEXT_PROFILE_MASK_ARB      0x9126
#define WGL_CONTEXT_CORE_PROFILE_BIT_ARB  0x00000001

#define K2Int(v) static_cast<int>(GameInputKey::v)
typedef HGLRC (WINAPI* ContextBuildFunc)(HDC, HGLRC, const int*);


static int s_win32KeyMap[KEYS_BUFFER_SIZE] = {};
static WindowParams* window;

void Window_InitializeKeyMap() {
    // K2K(Letters)
    s_win32KeyMap[0] = static_cast<int>(GameInputKey::KEY_UNKNOWN);
    s_win32KeyMap['A'] = K2Int(KEY_A);
    s_win32KeyMap['B'] = K2Int(KEY_B);
    s_win32KeyMap['C'] = K2Int(KEY_C);
    s_win32KeyMap['D'] = K2Int(KEY_D);
    s_win32KeyMap['E'] = K2Int(KEY_E);
    s_win32KeyMap['F'] = K2Int(KEY_F);
    s_win32KeyMap['G'] = K2Int(KEY_G);
    s_win32KeyMap['H'] = K2Int(KEY_H);
    s_win32KeyMap['I'] = K2Int(KEY_I);
    s_win32KeyMap['J'] = K2Int(KEY_J);
    s_win32KeyMap['K'] = K2Int(KEY_K);
    s_win32KeyMap['L'] = K2Int(KEY_L);
    s_win32KeyMap['M'] = K2Int(KEY_M);
    s_win32KeyMap['N'] = K2Int(KEY_N);
    s_win32KeyMap['O'] = K2Int(KEY_O);
    s_win32KeyMap['P'] = K2Int(KEY_P);
    s_win32KeyMap['Q'] = K2Int(KEY_Q);
    s_win32KeyMap['R'] = K2Int(KEY_R);
    s_win32KeyMap['S'] = K2Int(KEY_S);
    s_win32KeyMap['T'] = K2Int(KEY_T);
    s_win32KeyMap['U'] = K2Int(KEY_U);
    s_win32KeyMap['V'] = K2Int(KEY_V);
    s_win32KeyMap['W'] = K2Int(KEY_W);
    s_win32KeyMap['X'] = K2Int(KEY_X);
    s_win32KeyMap['Y'] = K2Int(KEY_Y);
    s_win32KeyMap['Z'] = K2Int(KEY_Z);

    // Numbers
    s_win32KeyMap['0'] = K2Int(KEY_0);
    s_win32KeyMap['1'] = K2Int(KEY_1);
    s_win32KeyMap['2'] = K2Int(KEY_2);
    s_win32KeyMap['3'] = K2Int(KEY_3);
    s_win32KeyMap['4'] = K2Int(KEY_4);
    s_win32KeyMap['5'] = K2Int(KEY_5);
    s_win32KeyMap['6'] = K2Int(KEY_6);
    s_win32KeyMap['7'] = K2Int(KEY_7);
    s_win32KeyMap['8'] = K2Int(KEY_8);
    s_win32KeyMap['9'] = K2Int(KEY_9);
    // Controls
    s_win32KeyMap[VK_ESCAPE]   = K2Int(KEY_ESCAPE);
    s_win32KeyMap[VK_SPACE]    = K2Int(KEY_SPACE);
    s_win32KeyMap[VK_RETURN]   = K2Int(KEY_ENTER);
    s_win32KeyMap[VK_TAB]      = K2Int(KEY_TAB);
    s_win32KeyMap[VK_BACK]     = K2Int(KEY_BACKSPACE);
    s_win32KeyMap[VK_LSHIFT]   = K2Int(KEY_LSHIFT);
    s_win32KeyMap[VK_RSHIFT]   = K2Int(KEY_RSHIFT);
    s_win32KeyMap[VK_LCONTROL] = K2Int(KEY_LCTRL);
    s_win32KeyMap[VK_RCONTROL] = K2Int(KEY_RCTRL);
    s_win32KeyMap[VK_LMENU]    = K2Int(KEY_LALT);
    s_win32KeyMap[VK_RMENU]    = K2Int(KEY_RALT);
    // Arrows
    s_win32KeyMap[VK_LEFT]  = K2Int(KEY_LEFT);
    s_win32KeyMap[VK_UP]    = K2Int(KEY_UP);
    s_win32KeyMap[VK_RIGHT] = K2Int(KEY_RIGHT);
    s_win32KeyMap[VK_DOWN]  = K2Int(KEY_DOWN);
}


void Win32WindowUpdate(const WindowParams& winParams)
{
    POINT mousePos;
    if (GetCursorPos(&mousePos)) {
        Input_SetAbsolutePosition(mousePos.x, mousePos.y);
        ScreenToClient(winParams.hwnd, &mousePos);
        mousePos.y = -(mousePos.y - winParams.height);
        if (mousePos.x < 0)
            mousePos.x = 0;
        if (mousePos.y < 0)
            mousePos.y = 0;
        Input_SetMousePosition(mousePos.x, mousePos.y);
    }
    MSG msg;
    while (PeekMessage(&msg, winParams.hwnd, 0, 0, PM_REMOVE)) {
        // std::cout << "event polled" << std::endl;
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
}


LRESULT CALLBACK WinProcFunc(HWND hwnd,
    UINT msg,
    WPARAM wParam,
    LPARAM lParam)
{
    //return DefWindowProc(hwnd, msg, wParam, lParam);
    if (window == nullptr) {
        return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    // std::cout << "Message Gotten: " << msg << std::endl;
    switch (msg) {
        case WM_CREATE:
            break;
        case WM_CLOSE:
            if (window->callbackClose != nullptr)
                window->callbackClose();
            break;
        case WM_SIZE:
            if (window->callbackResize != nullptr)
                window->callbackResize(LOWORD(lParam), HIWORD(lParam));
            break;
        case WM_MOVE:
            if (window->callbackMove != nullptr)
                window->callbackMove(LOWORD(lParam), HIWORD(lParam));
            break;

        case WM_DESTROY:
            if (window->callbackDestroy != nullptr)
                window->callbackDestroy();
            break;
        case WM_KEYDOWN: {
            if (wParam < KEYS_BUFFER_SIZE) {
                int key = s_win32KeyMap[wParam];
                Input_SetKeyState(key, true);
            }
            break;
        }
        case WM_KEYUP: {
            if (wParam < KEYS_BUFFER_SIZE) {
                int key = s_win32KeyMap[wParam];
                Input_SetKeyState(key, false);
            }
            break;
        }
        case WM_LBUTTONDOWN:
            Input_SetKeyState(K2Int(MOUSE_BUTTON_LEFT), true);
            break;
        case WM_LBUTTONUP:
            Input_SetKeyState(K2Int(MOUSE_BUTTON_LEFT), false);
            break;
        case WM_RBUTTONDOWN:
            Input_SetKeyState(K2Int(MOUSE_BUTTON_RIGHT), true);
            break;
        case WM_RBUTTONUP:
            Input_SetKeyState(K2Int(MOUSE_BUTTON_RIGHT), false);
            break;
        case WM_MBUTTONDOWN:
            Input_SetKeyState(K2Int(MOUSE_BUTTON_MIDDLE), true);
            break;
        case WM_MBUTTONUP:
            Input_SetKeyState(K2Int(MOUSE_BUTTON_MIDDLE), false);
            break;
        default:
            return DefWindowProc(hwnd, msg, wParam, lParam);
    }
    return (LRESULT)0;
}


void* GetAnyGLFuncAddress(const char *name) {
    void *p = (void *)wglGetProcAddress(name);
    if (p == nullptr || (p == (void*)0x1) || (p == (void*)0x2) || (p == (void*)0x3) || (p == (void*)-1))
    {
        HMODULE module = GetModuleHandleA("opengl32.dll");
        p = (void *)GetProcAddress(module, name);
    }
    return p;
}


void GetPixleFormatWin32(PIXELFORMATDESCRIPTOR& formatDescriptor) {
    formatDescriptor.nSize = sizeof(PIXELFORMATDESCRIPTOR);

}

bool CreateFirstWindowAndInitGL(WindowParams* paramsPtr){
    WindowParams& params = *paramsPtr;
    window = paramsPtr;

    Window_InitializeKeyMap();
    const char* mainWinClass = "mainWin";
    const char* dummyclassName = "dummyclass";
    WNDCLASSEX dummyWC = {
        sizeof(WNDCLASSEX),
        CS_OWNDC, DefWindowProc,
        0, 0,
        GetModuleHandle(nullptr),
        nullptr, nullptr, nullptr, nullptr,
        dummyclassName, nullptr
    };
    RegisterClassEx(&dummyWC);

    HWND dummyWnd = CreateWindowEx(0, dummyclassName, "no_name", 0,1,1,
        16, 16,
        nullptr, nullptr,
        params.hInst, nullptr);
    HDC dummyDc = GetDC(dummyWnd);

    // Pixel format struct to connect openGL context with win32.window
    PIXELFORMATDESCRIPTOR formatDescriptor =
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,
        PFD_DRAW_TO_WINDOW | PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER, // Flags
        PFD_TYPE_RGBA,  // The kind of framebuffer. RGBA or palette.
        32,             // Colordepth of the framebuffer.
        0, 0, 0, 0, 0, 0,
        0,
        0,
        0,
        0, 0, 0, 0,
        24,  // Number of bits for the depth-buffer
        8,   // Number of bits for the stencil-buffer
        0,   // Number of Aux buffers in the framebuffer.
        PFD_MAIN_PLANE,
        0,
        0, 0, 0
    };

    int win32PixelFormat = ChoosePixelFormat(dummyDc, &formatDescriptor);
    SetPixelFormat(dummyDc, win32PixelFormat, &formatDescriptor);

    HGLRC dummyRC = wglCreateContext(dummyDc);
    wglMakeCurrent(dummyDc, dummyRC);

    // Modern context creation Function
    auto contextBuilderFunc
    = (ContextBuildFunc)wglGetProcAddress("wglCreateContextAttribsARB");

    // Destroying temporary dummy window
    wglMakeCurrent(dummyDc, nullptr);
    wglDeleteContext(dummyRC);
    ReleaseDC(dummyWnd, dummyDc);
    DestroyWindow(dummyWnd);
    UnregisterClass(dummyclassName, params.hInst);

    if (contextBuilderFunc == nullptr) {
        std::cerr << "FAILED TO AQUIRE 'contextBuilderFunc'\n";
        return false;
    }

    // --- REAL MAIN WINDOW CREATION ---
    WNDCLASSEX wc = {
        sizeof(WNDCLASSEX),
        CS_OWNDC, WinProcFunc,
        0, 0,
        params.hInst,
        nullptr, nullptr, nullptr, nullptr,
        mainWinClass, nullptr
    };

    DWORD windowStyle = WS_OVERLAPPEDWINDOW
    | WS_CLIPSIBLINGS
    | WS_CLIPCHILDREN
    | WS_MAXIMIZEBOX
    | WS_MINIMIZEBOX
    | WS_SIZEBOX;

    RegisterClassEx(&wc);
    HWND mainHwnd = CreateWindowEx(
            0, mainWinClass, params.name, windowStyle,
            params.posX, params.posY,
            params.width, params.height,
            nullptr, nullptr, params.hInst, nullptr);

    if (mainHwnd == nullptr) {
        std::cerr << "FAILED HWND 'mainHwnd'\n";
        return false;
    }
    HDC mainDc = GetDC(mainHwnd);
    if (mainDc == nullptr) {
        std::cerr << "FAILED DC 'mainDC'\n";
        return false;
    }

    int mainPixelFormatWin32 = ChoosePixelFormat(mainDc, &formatDescriptor);
    SetPixelFormat(mainDc, mainPixelFormatWin32, &formatDescriptor);

    // Define OpenGL version as a [OPENGL_VERSION_MAJ, OPENGL_VERSION_MIN]
    int contextAttribs[] = {
        WGL_CONTEXT_MAJOR_VERSION_ARB, GL_VER_MAJOR,
        WGL_CONTEXT_MINOR_VERSION_ARB, GL_VER_MINOR,
        WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
        0
    };
    HGLRC mainRC = contextBuilderFunc(mainDc, NULL, contextAttribs);
    wglMakeCurrent(mainDc, mainRC);

    std::cout << "Calling to init glad\n";
    if (gladLoadGLLoader((GLADloadproc)GetAnyGLFuncAddress) == 0) {
        std::cerr << "Failed to initialize GLAD" << std::endl;
        return false;
    }
    params.dc = mainDc;
    params.glRc = mainRC;
    params.hwnd = mainHwnd;

    ShowWindow(mainHwnd, SW_SHOW);
    return true;
}