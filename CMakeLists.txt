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
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND UNIX)
    target_link_options(Terraria PRIVATE -Wl,--hash-style=both)
endif()

target_compile_options(Terraria PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

option(BUILD_TESTS "Build test executables" OFF)
if (BUILD_TESTS)
    file(GLOB_RECURSE TEST_SOURCES tests/*.cpp)
    file(GLOB_RECURSE TEST_LIB_SOURCES
        src/items/Inventory.cpp
        src/items/ItemDefinition.cpp
        src/world/TileRegistry.cpp
        src/crafting/Recipe.cpp
    )

    add_executable(test_inventory
        tests/test_inventory.cpp
        src/items/Inventory.cpp
        src/items/ItemDefinition.cpp
        src/world/TileRegistry.cpp
    )
    target_include_directories(test_inventory PRIVATE src)
    target_link_libraries(test_inventory PRIVATE raylib)

    add_executable(test_recipes
        tests/test_recipes.cpp
        src/crafting/Recipe.cpp
        src/items/ItemDefinition.cpp
        src/items/Inventory.cpp
        src/world/TileRegistry.cpp
    )
    target_include_directories(test_recipes PRIVATE src)
    target_link_libraries(test_recipes PRIVATE raylib)

    add_executable(test_item_definitions
        tests/test_item_definitions.cpp
        src/items/ItemDefinition.cpp
        src/world/TileRegistry.cpp
    )
    target_include_directories(test_item_definitions PRIVATE src)
    target_link_libraries(test_item_definitions PRIVATE raylib)
endif()
