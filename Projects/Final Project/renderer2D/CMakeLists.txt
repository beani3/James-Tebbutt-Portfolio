# CMakeList.txt follows same structure as one provided in GAMR3531

cmake_minimum_required(VERSION 3.10...3.29)

set(RENDERER_NAME "renderer2D")

# Library Files
set (RENDERER_HEADER_FILES
	"include/components/renderComponent.hpp"
	"include/components/transformComponent.hpp"
	"include/components/animationComponent.hpp"
	"include/components/physicsComponent.hpp"
	"include/components/tilemapComponent.hpp"
	"include/rendering/renderer.hpp"
	"include/rendering/camera.hpp"
	"include/windows/window.hpp"
	"include/core/input.hpp"
	"include/renderer2D.hpp"
)

set (RENDERER_SOURCE_FILES
	"src/rendering/renderer.cpp"
	"src/windows/window.cpp"
	"src/core/input.cpp"
)

# Add library target (renderer) and include directory
add_library(${RENDERER_NAME} ${RENDERER_SOURCE_FILES} ${RENDERER_HEADER_FILES})
target_include_directories(${RENDERER_NAME} PUBLIC "${CMAKE_CURRENT_SOURCE_DIR}/include")
# Visual Studio filters
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/src" PREFIX "src" FILES ${RENDERER_SOURCE_FILES})
source_group(TREE "${CMAKE_CURRENT_SOURCE_DIR}/include" PREFIX "include" FILES ${RENDERER_HEADER_FILES})

# Add external libraries
include(FetchContent)

find_package(OpenGL REQUIRED)

set(GLM_ENABLE_CXX_20 ON CACHE BOOL "" FORCE)
set(GLM_BUILD_LIBRARY ON CACHE BOOL "" FORCE)

FetchContent_Declare(glm
	GIT_REPOSITORY	https://github.com/g-truc/glm.git
	GIT_TAG 		1.0.1
)

FetchContent_Declare(glfw 
	GIT_REPOSITORY	https://github.com/glfw/glfw.git 
	GIT_TAG			3.4
)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)

FetchContent_Declare(glad
  	GIT_REPOSITORY https://github.com/SimonCouplandDMU/DMUGlad.git
  	GIT_TAG        gl4.6_core_ARB_bindless_texure
 )

FetchContent_Declare(tracy
	GIT_REPOSITORY https://github.com/wolfpld/tracy.git 
	GIT_TAG v0.13.1
	GIT_SHALLOW TRUE
	GIT_PROGRESS TRUE
)

FetchContent_Declare(entt
	GIT_REPOSITORY https://github.com/skypjack/entt.git
	GIT_TAG	v3.13.2
)


FetchContent_Declare(stb
  GIT_REPOSITORY https://github.com/nothings/stb.git
  GIT_TAG master
)

# Platformer specific

FetchContent_Declare(box2d
  GIT_REPOSITORY https://github.com/erincatto/box2d.git
  GIT_TAG v3.1.1
)

FetchContent_Declare(nlohmann
  GIT_REPOSITORY https://github.com/nlohmann/json.git
  GIT_TAG v3.11.3
)

FetchContent_MakeAvailable(glm glfw glad tracy entt stb box2d nlohmann)

# Override Tracy's default settings
message(STATUS "TRACY_ON_DEMAND: ON (On-demand profiling)")
target_compile_definitions(TracyClient PUBLIC TRACY_ON_DEMAND)
message(STATUS "TRACY_ONLY_LOCALHOST: ON (Only listen on the localhost interface)")
target_compile_definitions(TracyClient PUBLIC TRACY_ONLY_LOCALHOST)
message(STATUS "TRACY_NO_BROADCAST: ON (Disable client discovery by broadcast to local network)")
target_compile_definitions(TracyClient PUBLIC TRACY_ONLY_LOCALHOST)

# IMGUI needs a bit more fiddling with...  

set(IMGUI_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../build/_deps/imgui-")
set(IMGUI_SOURCE_DIR "${IMGUI_ROOT}src")
FetchContent_Populate(imgui
  URL https://github.com/ocornut/imgui/archive/refs/tags/v1.90.4-docking.zip
  SOURCE_DIR "${IMGUI_ROOT}src"
  SUBBUILD_DIR "${IMGUI_ROOT}subbuild"
  BINARY_DIR "${IMGUI_ROOT}build"
)

add_library(imgui_glfw STATIC
  ${IMGUI_SOURCE_DIR}/imgui.cpp
  ${IMGUI_SOURCE_DIR}/imgui_draw.cpp
  ${IMGUI_SOURCE_DIR}/imgui_demo.cpp
  ${IMGUI_SOURCE_DIR}/imgui_tables.cpp
  ${IMGUI_SOURCE_DIR}/imgui_widgets.cpp
  ${IMGUI_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
  ${IMGUI_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
)

set_target_properties(imgui_glfw PROPERTIES
	ARCHIVE_OUTPUT_DIRECTORY "${IMGUI_ROOT}build"
)

target_link_libraries(imgui_glfw PRIVATE glfw OpenGL::GL)

target_include_directories(imgui_glfw
PUBLIC
  ${IMGUI_SOURCE_DIR}
  ${IMGUI_SOURCE_DIR}/backends
)

# Add and create STB image files specifically
set(STB_IMAGE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../build/_deps/stb_image")
file(DOWNLOAD https://raw.githubusercontent.com/nothings/stb/master/stb_image.h  "${STB_IMAGE_DIR}/stbImage/stb_image.h" STATUS _stat TLS_VERIFY ON LOG _log)
#message("status  ${_stat} log ${_log}")

file(WRITE "${STB_IMAGE_DIR}/stbImage/stb_image.cpp" "#define STB_IMAGE_IMPLEMENTATION\n#include \"stb_image.h\"")

add_library(stb_image_impl STATIC "${STB_IMAGE_DIR}/stbImage/stb_image.cpp")

target_include_directories(stb_image_impl PUBLIC "${STB_IMAGE_DIR}")

# Use C++20 everywhere
set_target_properties(${Prototype_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(${RENDERER_NAME} PROPERTIES CXX_STANDARD 20)
set_target_properties(glm PROPERTIES CXX_STANDARD 20)
set_target_properties(glfw PROPERTIES CXX_STANDARD 20)
set_target_properties(glad PROPERTIES CXX_STANDARD 20)
set_target_properties(imgui_glfw PROPERTIES CXX_STANDARD 20)
set_target_properties(stb_image_impl PROPERTIES CXX_STANDARD 20)
set_target_properties(TracyClient PROPERTIES CXX_STANDARD 20)
set_target_properties(EnTT PROPERTIES CXX_STANDARD 20)
set_target_properties(box2d PROPERTIES CXX_STANDARD 20)
set_target_properties(nlohmann_json PROPERTIES CXX_STANDARD 20)

target_link_libraries(${RENDERER_NAME} PUBLIC OpenGL::GL)
target_link_libraries(${RENDERER_NAME} PUBLIC glm)
target_link_libraries(${RENDERER_NAME} PUBLIC glfw)
target_link_libraries(${RENDERER_NAME} PUBLIC glad)
target_link_libraries(${RENDERER_NAME} PUBLIC imgui_glfw)
target_link_libraries(${RENDERER_NAME} PUBLIC stb_image_impl)
target_link_libraries(${RENDERER_NAME} PUBLIC TracyClient)
target_link_libraries(${RENDERER_NAME} PUBLIC EnTT)
target_link_libraries(${RENDERER_NAME} PUBLIC box2d)
target_link_libraries(${RENDERER_NAME} PUBLIC nlohmann_json)

set_target_properties(${RENDERER_NAME} PROPERTIES FOLDER "renderer2D")

set_target_properties(glad PROPERTIES FOLDER "renderer2D/Deps")

set_target_properties(glfw PROPERTIES FOLDER "renderer2D/Deps/GLFW")
set_target_properties(uninstall PROPERTIES FOLDER "renderer2D/Deps/GLFW")
set_target_properties(update_mappings PROPERTIES FOLDER "renderer2D/Deps/GLFW")

set_target_properties(imgui_glfw PROPERTIES FOLDER "renderer2D/Deps")

set_target_properties(glm PROPERTIES FOLDER "renderer2D/Deps")

set_target_properties(TracyClient PROPERTIES FOLDER "renderer2D/Deps")

set_target_properties(stb_image_impl PROPERTIES FOLDER "renderer2D/Deps")

set_target_properties(nlohmann_json PROPERTIES FOLDER "renderer2D/Deps")
