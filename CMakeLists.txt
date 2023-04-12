cmake_minimum_required(VERSION 3.25)
project(nesemu_cpp)

set(CMAKE_CXX_STANDARD 20)

add_executable(nesemu_cpp
        src/sokol_app.h
        src/sokol_audio.h
        src/sokol_gfx.h
        src/sokol_glue.h
        src/sokol_log.h
        src/NES.h
        src/NES.cpp
        src/cpu.cpp
        src/memory.cpp
        src/main.cpp
        )
