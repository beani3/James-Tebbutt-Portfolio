# CMakeList.txt follows same structure as one provided in GAMR3531

set(Demo_NAME "finalProject")

project(${Demo_NAME} VERSION 1.0.0)

# Demo Files

# Shader Files
set (PROJECT_SHADER_FILES
	"assets/shaders/texturedQuadVert.glsl"
	"assets/shaders/texturedQuadFrag.glsl"
)

# Header Files
set (PROJECT_HEADER_FILES
	"include/core/application.hpp"
	"include/rendering/sceneLoader.hpp"
	"include/finalProject.hpp"
)

# Source Files
set (PROJECT_SOURCE_FILES
	"src/core/application.cpp"
	"src/rendering/sceneLoader.cpp"
	"src/finalProject.cpp"
	"src/main.cpp"
)

# Add excutable target and include directory
add_executable(${Demo_NAME} ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES})
target_include_directories(${Demo_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${Demo_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/finalProject/include")

# Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${PROJECT_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${PROJECT_HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders" PREFIX "shaders" FILES ${PROJECT_SHADER_FILES})

# Add the shader files to the target so they appear in Visual Studio
target_sources(${Demo_NAME} PRIVATE ${PROJECT_SHADER_FILES})

# Use C++20
set_target_properties(${Demo_NAME} PROPERTIES CXX_STANDARD 20)

target_link_libraries(${Demo_NAME} PRIVATE "renderer2D")

set_target_properties(${Demo_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})