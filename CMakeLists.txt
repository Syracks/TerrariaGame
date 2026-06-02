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

set(SOURCES
    src/app/Game.cpp
    src/app/GameRenderer.cpp
    src/app/CameraController.cpp
    src/app/GameSession.cpp
    src/app/MobSpawner.cpp
    src/app/PlayerController.cpp
    src/core/SoundManager.cpp
    src/core/MusicManager.cpp
    src/core/TextureManager.cpp
    src/crafting/Recipe.cpp
    src/entities/Mob.cpp
    src/entities/Player.cpp
    src/items/Inventory.cpp
    src/items/ItemDefinition.cpp
    src/items/Tool.cpp
    src/main.cpp
    src/save/SaveManager.cpp
    src/systems/CollisionSystem.cpp
    src/systems/CombatSystem.cpp
    src/systems/DeathSystem.cpp
    src/systems/InteractionSystem.cpp
    src/systems/LiquidSystem.cpp
    src/systems/MiningSystem.cpp
    src/systems/ParticleSystem.cpp
    src/systems/PhysicsSystem.cpp
    src/systems/RenderSystem.cpp
    src/ui/HUD.cpp
    src/ui/InventoryScreen.cpp
    src/ui/MainMenu.cpp
    src/ui/Minimap.cpp
    src/ui/SettingsMenu.cpp
    src/ui/ChestScreen.cpp
    src/world/Chunk.cpp
    src/world/TileRegistry.cpp
    src/world/World.cpp
    src/world/WorldGenerator.cpp
)

set(TEST_LIB_SOURCES
    src/items/Inventory.cpp
    src/items/ItemDefinition.cpp
    src/world/TileRegistry.cpp
    src/crafting/Recipe.cpp
)

add_executable(Terraria ${SOURCES})

target_include_directories(Terraria PRIVATE src)
target_link_libraries(Terraria PRIVATE raylib)
if (CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang" AND UNIX AND NOT APPLE)
    target_link_options(Terraria PRIVATE -Wl,--hash-style=both)
endif()

target_compile_options(Terraria PRIVATE
    $<$<CXX_COMPILER_ID:GNU,Clang>:-Wall -Wextra -Wpedantic>
    $<$<CXX_COMPILER_ID:MSVC>:/W4>
)

option(BUILD_TESTS "Build test executables" OFF)
if (BUILD_TESTS)
    enable_testing()

    add_library(TestLib STATIC ${TEST_LIB_SOURCES})
    target_include_directories(TestLib PUBLIC src)
    target_link_libraries(TestLib PUBLIC raylib)

    add_executable(test_inventory tests/test_inventory.cpp)
    target_link_libraries(test_inventory PRIVATE TestLib)
    add_test(NAME test_inventory COMMAND test_inventory)

    add_executable(test_recipes tests/test_recipes.cpp)
    target_link_libraries(test_recipes PRIVATE TestLib)
    add_test(NAME test_recipes COMMAND test_recipes)

    add_executable(test_item_definitions tests/test_item_definitions.cpp)
    target_link_libraries(test_item_definitions PRIVATE TestLib)
    add_test(NAME test_item_definitions COMMAND test_item_definitions)
endif()
