cmake_minimum_required(VERSION 4.2)
project(RenderGL C CXX)

set(CMAKE_CXX_STANDARD 23)

add_subdirectory("cglm")
add_subdirectory("stb")

include_directories("glad/include")
add_library(glad glad/src/glad.c)


add_executable(
        RenderGL main.cpp
        PlatformWin32.cpp
        Shader.cpp
        Application.cpp
        Application.h
        AssetManager.cpp
        AssetManager.h
        Texture.cpp
        Texture.h
        InputSystem.cpp
        InputSystem.h
        MyTypes.h
        GameTime.cpp
        GameTime.h
        Transform.cpp
        Transform.h
)

target_link_libraries(RenderGL glad opengl32 cglm stb_lib gdi32 winmm)
