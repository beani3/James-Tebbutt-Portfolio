# CMakeList.txt follows same structure as one provided in GAMR3531

cmake_minimum_required(VERSION 3.10...3.29)

project("Final Year Project" VERSION 1.0.0)

# Prevent windows.h min max macros upsetting std::min and std:max from algorithm.h
add_compile_definitions(NOMINMAX)

# Give access to the root CMakeLists.txt (this) file  and README in Solution explorer
add_custom_target(CMakeRoot SOURCES ${CMAKE_SOURCE_DIR}/CMakeLists.txt ${CMAKE_SOURCE_DIR}/README.md)

# Add renderer2D
add_subdirectory("renderer2D")

# Add finalProject
add_subdirectory("finalProject")

# Startup project
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT finalProject)