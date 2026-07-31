cmake_minimum_required(VERSION 4.2)
project(RenderGL C CXX)

set(CMAKE_CXX_STANDARD 23)

add_library(assimp SHARED IMPORTED)

set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/assimp/release/assimp-vc143-mt.dll")
set_target_properties(assimp PROPERTIES IMPORTED_LOCATION_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/assimp/debug/assimp-vc143-mt.dll")
set_target_properties(assimp PROPERTIES IMPORTED_IMPLIB_RELEASE "${CMAKE_CURRENT_SOURCE_DIR}/assimp/release/assimp-vc143-mt.lib")
set_target_properties(assimp PROPERTIES IMPORTED_IMPLIB_DEBUG "${CMAKE_CURRENT_SOURCE_DIR}/assimp/debug/assimp-vc143-mt.lib")
target_include_directories(assimp INTERFACE "${CMAKE_CURRENT_SOURCE_DIR}/assimp")

add_library(glad glad/src/glad.c)

add_subdirectory("cglm")
add_subdirectory("stb")
add_subdirectory("imgui")

include_directories("glad/include")
include_directories("imgui")



add_executable( RenderGL  WIN32  main.cpp
        Shader.h
        Shader.cpp
        AssetManager.h
        AssetManager.cpp
        Texture.cpp
        Texture.h
        InputSystem.cpp
        InputSystem.h
        MyTypes.h
        GameTime.cpp
        GameTime.h
        Transform.cpp
        Transform.h
        SlotsMap.h
        Mesh.h
        Mesh.cpp
        Light.h
        RenderObject.cpp
        RenderObject.h
        GameScene.cpp
        GameScene.h
        Handle.h
        GraphicsGL.cpp
        GraphicsGL.h
        Material.cpp
        Material.h
        RenderSubMesh.h
        Uniforms.h
        Skybox.h
        Camera.cpp
        Camera.h
        ShaderWatcher.h
        ShaderWatcher.cpp
        SceneDefinition.cpp
        SceneDefinition.h
        Engine.cpp
        Engine.h
        ProjectDefines.h
        ProjectSettings.h
        ProjectSettings.cpp
        vector4.h
        RenderMode.h
        InstancedBuffer.cpp
        InstancedBuffer.h
        terrain.cpp
        terrain.h
        NoiseGenerator.cpp
        NoiseGenerator.h
        uiObject.cpp
        uiObject.h
        NoiseData.h
        SplatMapData.h
        myGui.cpp
        myGui.h
        ProgramWindow.cpp
        ProgramWindow.h
)


target_link_libraries(RenderGL glad opengl32 cglm stb_lib gdi32 winmm assimp imgui_lib)


message(STATUS "POST BUILD 1...")
message(STATUS "POST BUILD 2... ${TARGET_RUNTIME_DLLS}")
message(STATUS "POST BUILD 3... ${TARGET_FILE_DIR}")

if (WIN32)
    add_custom_command(
            TARGET RenderGL POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "$<TARGET_RUNTIME_DLLS:RenderGL>"
            "$<TARGET_FILE_DIR:RenderGL>"
            COMMAND_EXPAND_LISTS
    )
endif()

