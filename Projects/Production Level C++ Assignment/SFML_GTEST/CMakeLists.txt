cmake_minimum_required(VERSION 3.28)

#project(SFML_GTEST)
set(PROJECT_NAME "SFML_GTEST")

# Use C++17 everywhere
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

set(PROJECT_HEADER_FILES
    "include/DynamicObject.h"
    "include/GameObject.h"
    "include/StaticObject.h"
    "include/AnimationManager.h"
    "include/EnemyNPC.h"
    "include/GameEngine.h"
    "include/PlayerCharacter.h"
    "include/Tilemap.h"
    "include/EnemyBuilder.h"
    "include/NPCBuilder.h"
    "include/SplashScreen.h"
)

set(PROJECT_SOURCE_FILES
    "src/Main.cpp"
    "src/DynamicObject.cpp"
    "src/StaticObject.cpp"
    "src/AnimationManager.cpp"
    "src/EnemyNPC.cpp"
    "src/GameEngine.cpp"
    "src/PlayerCharacter.cpp"
    "src/Tilemap.cpp"
    "src/SplashScreen.cpp"
)

set(TEST_SOURCE_FILES
    "src/testgtest.cpp"
    "src/DynamicObject.cpp"
    "src/StaticObject.cpp"
    "src/AnimationManager.cpp"
    "src/EnemyNPC.cpp"
    "src/GameEngine.cpp"
    "src/PlayerCharacter.cpp"
    "src/Tilemap.cpp"
    "src/SplashScreen.cpp"
)

add_executable(${PROJECT_NAME} ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})
add_executable(RunUnitTests ${TEST_SOURCE_FILES} ${PROJECT_HEADER_FILES})
target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(RunUnitTests PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")

source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${PROJECT_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${TEST_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${PROJECT_HEADER_FILES})

include(FetchContent)

# GoogleTest
FetchContent_Declare(
    googletest
    URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)

# SFML 2.6.2
FetchContent_Declare(
    SFML
    GIT_REPOSITORY https://github.com/SFML/SFML.git
    GIT_TAG 2.6.2
)

FetchContent_MakeAvailable(googletest SFML)

target_link_libraries(
    ${PROJECT_NAME}
    PRIVATE
        sfml-system
        sfml-window
        sfml-graphics
        sfml-network
        sfml-audio
)

target_link_libraries(
    RunUnitTests
    PRIVATE
        gtest
        gtest_main
        sfml-system
        sfml-window
        sfml-graphics
        sfml-network
        sfml-audio
)

enable_testing()

add_test(
    NAME RunUnitTests
    COMMAND RunUnitTests
)

install(
    TARGETS ${PROJECT_NAME}
    DESTINATION bin
)

install(
    FILES Main.cpp
    DESTINATION src
)

set(CMAKE_SUPPRESS_REGENERATION TRUE)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)