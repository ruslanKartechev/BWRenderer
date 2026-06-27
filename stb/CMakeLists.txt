cmake_minimum_required(VERSION 3.13)
project(stb_lib C)

set(CMAKE_C_STANDARD 11)
set(CMAKE_C_STANDARD_REQUIRED YES)
set(DEFAULT_BUILD_TYPE "Release")

add_library(stb_lib STATIC
        stb_image.cpp
        stb_image.h
        )

target_include_directories(stb_lib PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})
