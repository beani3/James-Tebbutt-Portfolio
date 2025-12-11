cmake_minimum_required(VERSION 3.28)
project(SFML_GTEST)

# GoogleTest requires at least C++14
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
#set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

# Add test cpp file
add_executable( RunUnitTests "testgtest.cpp" "DynamicObject.h" "DynamicObject.cpp" "GameObject.h" "StaticObject.h" 
"StaticObject.cpp" "AnimationManager.h" "AnimationManager.cpp" "EnemyNPC.h" "EnemyNPC.cpp" "GameEngine.h" "GameEngine.cpp"
"PlayerCharacter.h" "PlayerCharacter.cpp" "Tilemap.h" "Tilemap.cpp" "EnemyBuilder.h" "NPCBuilder.h" "SplashScreen.h" "SplashScreen.cpp")

#find_package(SFML 2.6.2 COMPONENTS graphics audio REQUIRED)
add_executable("${PROJECT_NAME}" 
"Main.cpp" "DynamicObject.h" "DynamicObject.cpp" "GameObject.h" "StaticObject.h" 
"StaticObject.cpp" "AnimationManager.h" "AnimationManager.cpp" "EnemyNPC.h" "EnemyNPC.cpp" "GameEngine.h" "GameEngine.cpp"
"PlayerCharacter.h" "PlayerCharacter.cpp" "Tilemap.h" "Tilemap.cpp" "EnemyBuilder.h" "NPCBuilder.h" "SplashScreen.h" "SplashScreen.cpp")

set(SFML_DIR "SFML-2.6.2/lib/cmake/SFML")

#look for SFML and add it
find_package (SFML 2.6.2 REQUIRED system window graphics network audio)

target_link_libraries(SFML_GTEST debug sfml-audio sfml-graphics sfml-network sfml-system gtest gtest_main)

# Link test executable against gtest & gtest_main
target_link_libraries(RunUnitTests gtest gtest_main sfml-audio sfml-graphics sfml-network sfml-system )
add_test( RunUnitTests RunUnitTests )

install(TARGETS "${PROJECT_NAME}" DESTINATION bin) #${CMAKE_INSTALL_PREFIX}/bin
install(FILES "Main.cpp" DESTINATION src)

# Group into "Source Files" and "Header Files"
    if ("${FILE}" MATCHES ".*\\.cpp")
      set(GROUP "src${GROUP}")
    elseif("${FILE}" MATCHES ".*\\.h")
      set(GROUP "include${GROUP}")
    endif()

	
set(CMAKE_SUPPRESS_REGENERATION true)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)