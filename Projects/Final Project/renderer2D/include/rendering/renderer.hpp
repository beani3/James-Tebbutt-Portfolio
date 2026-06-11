/** @file Renderer.hpp
 *  @brief Renderer abstraction for simple batched 2D textured quads.
 *
 *  The Renderer batches quads into SSBOs and issues instanced draws. It
 *  supports loading textures and obtaining bindless handles for use by shaders.
 */

#pragma once
#include <glm/glm.hpp>
#include "windows/window.hpp"
#include <vector>
#include <glad/glad.h>
#include <filesystem>
#include <entt/entt.hpp>
#include <rendering/camera.hpp>

using Scene = entt::registry;

/**
 * @brief CPU vertex representation mirrored in shader storage.
 */
struct Vertex {
	glm::vec4 colour; //!< RGBA colour of the vertex
	glm::vec2 position; //!< Position of the vertex in 2D space
	glm::vec2 uv; //!< Texture coordinates for the vertex
};

/**
 * @brief Simple descriptor used to load textures with stb_image.
 */
struct TextureDescription {
	const char* src{ "" }; //!< File path to image source
	int width{ 256 }; //!< Expected/initial width (stb_image may override)
	int height{ 256 }; //!< Expected/initial height (stb_image may override)
	int channels{ 4 }; //!< Expected channel count; 4 = RGBA

	int getWidth() { return width; }
	int getHeight() { return height; }
}; 

/**
 * @brief Minimal renderer that batches quads from RenderComponent entries.
 *
 * The renderer maintains CPU-side buffers holding vertices and texture handles,
 * uploads them to GPU SSBOs and draws using instanced rendering.
 */
class Renderer {
public:
	/** @brief Default constructor performs GL resource setup and shader compilation. */
	Renderer();

	/** @brief Default destructor releases resources (TODO: implement). */
	~Renderer();

	/**
	 * @brief Render the provided scene.
	 * @param scene entt registry containing entities with RenderComponent instances.
	 *
	 * The renderer iterates over RenderComponent components and builds a batch
	 * of vertices and texture handles which are flushed to the GPU.
	 */
	void render(Scene& scene);

	/**
	 * @brief Compile a shader from file and return the shader object id.
	 * @param shaderObj Existing shader object id (ignored; returned on success).
	 * @param src Filesystem path to shader source.
	 * @param shaderType GL shader type (e.g., GL_VERTEX_SHADER).
	 * @return GLuint shader object id.
	 * @throws std::runtime_error on shader creation failure.
	 */
	GLuint CompileShader(GLuint shaderObj, const std::filesystem::path& src, GLuint shaderType);

	/**
	 * @brief Load an image from disk into an OpenGL texture.
	 * @param texDesc TextureDescription describing the image to load.
	 * @return GLuint OpenGL texture id.
	 * @throws std::runtime_error if the image fails to load.
	 */
	GLuint loadTexture(TextureDescription texDesc);

	/**
	 * @brief Retrieve a bindless texture handle and make it resident.
	 * @param texture OpenGL texture id.
	 * @return GLuint64 bindless texture handle usable in shaders.
	 *
	 * Requires ARB_bindless_texture support.
	 */
	GLuint64 getHandle(GLuint texture);

	/** Getters */
	size_t getShaderID() const { return m_shaderID; } //!< Accessor for the shader program ID (for setting uniforms, etc.)
	size_t getUBO() const { return m_uboID; } //!< Accessor for the UBO ID (currently unused/reserved for future use)

private:
	/** @brief Prepare per-frame state and clear CPU buffers. */
	void begin();

	/** @brief Finish frame by flushing any remaining batches. */
	void end();

	/** @brief Upload CPU buffers to GPU and issue draw calls for the current batch. */
	void flush();

private:
	Camera m_camera; //!< Camera instance providing view and projection matrices for rendering
	std::vector<Vertex> m_cpuVertBuffer; //!< Stores vertex data passed in from SSBO
	std::vector<uint64_t> m_textureHandles; //!< Stores texture handles for bindless texture

	glm::vec2 m_viewportSize{ 0.f, 0.f };

	static const size_t s_batchSize{ 1024 }; //!< Limit to number of quads per batch
	static const size_t s_vertexCapacity{ 8096 * 4 }; //!< Limit to number of vertices per batch
	static const size_t s_textureCapacity{ 8096 }; //!< Limit to number of textures held by the renderer
	size_t m_currentQuadCount{ 0 }; //!< Track number of quads in the current batch

	GLuint m_shaderID{ 0 }; //!< Program id for textured quad shader
	GLuint m_vertSsboID{ 0 }; //!< SSBO id for vertex data
	GLuint m_texSsboID{ 0 }; //!< SSBO id for texture handle data
	GLuint m_uboID{ 0 }; //!< UBO id (reserved/unused currently)
	GLuint m_vaoID{ 0 }; //!< VAO id used for draw calls
};