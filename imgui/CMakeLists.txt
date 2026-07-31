# imgui/CMakeLists.txt
cmake_minimum_required(VERSION 3.20)
project(imgui_lib)

add_library(imgui_lib STATIC
        imgui.cpp
        imgui_draw.cpp
        imgui_tables.cpp
        imgui_widgets.cpp
        imgui_demo.cpp
        imgui_impl_opengl3.h
        imgui_impl_opengl3.cpp
        imgui_impl_opengl3_loader.h
        imgui_impl_win32.h
        imgui_impl_win32.cpp
)

target_include_directories(imgui_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
