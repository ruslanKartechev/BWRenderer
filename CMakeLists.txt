cmake_minimum_required(VERSION 4.2)
project(RenderGL C CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(assimp STATIC IMPORTED)
set_target_properties(assimp PROPERTIES IMPORTED_LOCATION "${CMAKE_CURRENT_SOURCE_DIR}/assimp/lib/assimp-vc143-mt.lib")
target_include_directories(assimp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/assimp")

add_subdirectory("cglm")
add_subdirectory("stb")

include_directories("glad/include")
add_library(glad glad/src/glad.c)


add_executable( RenderGL  WIN32  main.cpp
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
        DataStructures.h
        Slot.h
        Mesh.h
)


target_link_libraries(RenderGL glad opengl32 cglm stb_lib gdi32 winmm assimp)
