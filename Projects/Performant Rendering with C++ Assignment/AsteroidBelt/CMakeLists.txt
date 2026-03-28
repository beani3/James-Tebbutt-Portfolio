set(APPLICATION_NAME "AsteroidBelt")

project(${APPLICATION_NAME} VERSION 1.0.0)

# Application Files

# Shader Files
set (PROJECT_SHADER_FILES
	"assets/shaders/Bloom/debugVert.glsl"
	"assets/shaders/Bloom/debugFrag.glsl"
	"assets/shaders/Bloom/Vert.glsl"
	"assets/shaders/Bloom/ThresholdFrag.glsl"
	"assets/shaders/Bloom/DownBlurFrag.glsl"
	"assets/shaders/Bloom/upFilterFrag.glsl"
	"assets/shaders/Composition/Vert.glsl"
	"assets/shaders/Composition/Frag.glsl"
	"assets/shaders/Skybox/Vert.glsl"
	"assets/shaders/Skybox/Frag.glsl"
	"assets/shaders/PBR/pbrVertex.glsl"
	"assets/shaders/PBR/pbrFrag.glsl"
	"assets/shaders/PBR/pbrFragEmissive.glsl"
	"assets/shaders/Phong/Vert.glsl"
	"assets/shaders/Phong/EmissiveFrag.glsl"
	"assets/shaders/UI/QuadVert.glsl"
	"assets/shaders/UI/QuadFrag.glsl"
	"assets/shaders/UI/CircleVert.glsl"
	"assets/shaders/Pre-Pass/depthVert.glsl"
	"assets/shaders/Pre-Pass/depthFrag.glsl"
)

# Header Files
set (PROJECT_HEADER_FILES
	"include/app.hpp"
	"include/gameLayer.hpp"
	"include/uiLayer.hpp"
	"include/sharedData.hpp"
	"include/ImGui/bloomPanel.hpp"
	"include/ImGui/textureSlotPanel.hpp"
	"include/LOD.hpp"
)

# Source Files
set (PROJECT_SOURCE_FILES
	"src/app.cpp"
	"src/gameLayer.cpp"
	"src/uiLayer.cpp"
	"src/ImGui/bloomPanel.cpp"
	"src/ImGui/textureSlotPanel.cpp"
	"src/LOD.cpp"
)

# YOU WILL NEED TO ADD YOUR SCRIPT FILES HERE
set (SCRIPT_HEADER_FILES
	"scripts/include/camera.hpp"
	"scripts/include/followCamera.hpp"
	"scripts/include/controller.hpp"
	"scripts/include/rotation.hpp"
)
set (SCRIPT_SOURCE_FILES
	"scripts/src/camera.cpp"
	"scripts/src/followCamera.cpp"
	"scripts/src/controller.cpp"
	"scripts/src/rotation.cpp"
)

# Add excutable target and include directory
add_executable(${APPLICATION_NAME} ${PROJECT_SOURCE_FILES} ${PROJECT_HEADER_FILES} ${SCRIPT_SOURCE_FILES} ${SCRIPT_HEADER_FILES})
target_include_directories(${APPLICATION_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/include")
target_include_directories(${APPLICATION_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/DemonRenderer/include")
target_include_directories(${APPLICATION_NAME} PRIVATE "${CMAKE_CURRENT_SOURCE_DIR}/scripts/include/") # For scripts, might change this

# Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${PROJECT_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${PROJECT_HEADER_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/scripts" PREFIX "gameScripts" FILES ${SCRIPT_HEADER_FILES} ${SCRIPT_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders" PREFIX "shaders" FILES ${PROJECT_SHADER_FILES})

# Add the shader files to the target so they appear in Visual Studio
target_sources(${APPLICATION_NAME} PRIVATE ${PROJECT_SHADER_FILES})

# Use C++20
set_target_properties(${APPLICATION_NAME} PROPERTIES CXX_STANDARD 20)

target_link_libraries(${APPLICATION_NAME} PRIVATE "DemonRenderer")

set_target_properties(${APPLICATION_NAME} PROPERTIES VS_DEBUGGER_WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR})
