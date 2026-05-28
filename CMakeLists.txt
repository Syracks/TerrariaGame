cmake_minimum_required(VERSION 3.20)
project(Terraria LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    raylib
    GIT_REPOSITORY https://github.com/raysan5/raylib.git
    GIT_TAG 5.5
)

FetchContent_MakeAvailable(raylib)

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(Terraria ${SOURCES})

target_include_directories(Terraria PRIVATE src)
target_link_libraries(Terraria PRIVATE raylib)
