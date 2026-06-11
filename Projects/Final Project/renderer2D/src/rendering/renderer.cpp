/**
 * @file renderer.cpp
 * @brief Implementation of the Renderer class responsible for batched 2D rendering.
 *
 * The Renderer collects quad data from `RenderComponent` instances held in an
 * entt::registry, uploads vertex + texture handle buffers to GPU-side SSBOs and
 * issues instanced draws. It also provides utility helpers for compiling GLSL
 * shaders, loading textures via stb_image and creating bindless texture handles.
 *
 * Threading / context:
 * - All GL calls assume a valid OpenGL context is current on the calling thread.
 * - ARB_bindless_texture functions are used when acquiring handles; ensure the
 *   extension is available at runtime before calling `getHandle`.
 */

#include "rendering/renderer.hpp"
#include <components/renderComponent.hpp>
#include <components/transformComponent.hpp>
#include <components/physicsComponent.hpp>
#include <components/animationComponent.hpp>
#include "windows/window.hpp"
#include <stdexcept>
#include <sstream>
#include <fstream>
#include <glm/ext/matrix_clip_space.hpp>
#include "stbImage/stb_image.h"
#include <iostream>
#include <glm/gtc/type_ptr.hpp>

/**
 * @brief Construct and initialise GL resources, SSBOs and shaders.
 *
 * Detailed behaviour:
 * - Creates and binds a VAO used for draw calls.
 * - Allocates GPU storage for two SSBOs sized to the renderer's reserved CPU buffers.
 * - Compiles vertex and fragment shaders from disk and links them into a program.
 * - Deletes the temporary shader objects after linking.
 *
 * @throws std::runtime_error when shader creation (glCreateShader) fails inside CompileShader.
 *
 * @note Shader source paths are hard-coded relative to the application working directory.
 */
Renderer::Renderer() {
	glClearColor(0.f, 0.f, 0.f, 1.f); //!< Set clear colour 

	/** Bind VAO */
	glCreateVertexArrays(1, &m_vaoID);
	glBindVertexArray(m_vaoID);

	/** Vertex data SSBO setup */
	m_cpuVertBuffer.reserve(s_vertexCapacity);
	glCreateBuffers(1, &m_vertSsboID);
	glNamedBufferStorage(m_vertSsboID, sizeof(Vertex) * m_cpuVertBuffer.capacity(), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vertSsboID); //!< Bind VBO to SSBO binding point 0 for shader access

	m_textureHandles.reserve(s_textureCapacity);
	glCreateBuffers(1, &m_texSsboID);
	glNamedBufferStorage(m_texSsboID, sizeof(uint64_t) * m_textureHandles.capacity(), nullptr, GL_DYNAMIC_STORAGE_BIT);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_texSsboID); //!< Bind VBO to SSBO binding point 1 for shader access

	/** Bind Vertex Shader */
	GLuint vs = 0;
	const std::filesystem::path vsSrc = "./assets/shaders/texturedQuadVert.glsl";
	vs = CompileShader(vs, vsSrc, GL_VERTEX_SHADER);

	/** Bind Fragment Shader */
	GLuint fs = 0;
	const std::filesystem::path fsSrc = "./assets/shaders/texturedQuadFrag.glsl"; 
	fs = CompileShader(fs, fsSrc, GL_FRAGMENT_SHADER);

	/** Create shader program and attach VS and FS */
	m_shaderID = glCreateProgram();
	glAttachShader(m_shaderID, vs);
	glAttachShader(m_shaderID, fs);
	glLinkProgram(m_shaderID);

	GLint isLinked = 0;
	glGetProgramiv(m_shaderID, GL_LINK_STATUS, (int*)&isLinked);
	if (isLinked == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetProgramiv(m_shaderID, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetProgramInfoLog(m_shaderID, maxLength, &maxLength, &infoLog[0]);
		std::cout << "Shader linking error: {0} " << std::string(infoLog.begin(), infoLog.end()) << std::endl;

		glDeleteProgram(m_shaderID);
		m_shaderID = 0;
	}

	/** We no longer need the separate shader objects once program is linked : */
	if (vs != 0) {
		if (m_shaderID != 0) glDetachShader(m_shaderID, vs);
		glDeleteShader(vs);
	}
	if (fs != 0) {
		if (m_shaderID != 0) glDetachShader(m_shaderID, fs);
		glDeleteShader(fs);
	}

	glUseProgram(m_shaderID);
}

/**
 * @brief Destructor releases GL resources owned by the Renderer.
 *
 * Actions performed:
 * - Makes any tracked bindless texture handles non-resident (if ARB_bindless_texture is present).
 * - Unbinds the current program.
 * - Deletes the GL program, SSBOs, UBO (if created) and VAO.
 *
 * @note The actual GL texture objects returned by `loadTexture` are owned by the caller
 *       and are not deleted here.
 */
Renderer::~Renderer() {
	// Make tracked bindless texture handles non-resident
	for (auto handle : m_textureHandles) {
		if (handle != 0) {
			glMakeTextureHandleNonResidentARB(handle);
		}
	}
	m_textureHandles.clear();

	glUseProgram(0); //!< Unbind program

	if (m_shaderID != 0) { //!< Delete GL program
		glDeleteProgram(m_shaderID);
		m_shaderID = 0;
	}

	if (m_vertSsboID != 0) { //!< Delete SSBOs / UBOs
		glDeleteBuffers(1, &m_vertSsboID);
		m_vertSsboID = 0;
	}
	if (m_texSsboID != 0) {
		glDeleteBuffers(1, &m_texSsboID);
		m_texSsboID = 0;
	}
	if (m_uboID != 0) {
		glDeleteBuffers(1, &m_uboID);
		m_uboID = 0;
	}

	if (m_vaoID != 0) { //!< Delete VAO
		glDeleteVertexArrays(1, &m_vaoID);
		m_vaoID = 0;
	}
}

/**
 * @brief Build batches from scene RenderComponent + TransformComponent entries and draw.
 *
 * Iterates all entities with both a `RenderComponent` and `TransformComponent`.
 * For each entity the quad's four vertices are generated from `pos` and `halfExtent`,
 * transformed by the entity's transform matrix and placed into the CPU-side vertex
 * buffer. Texture handles are stored in parallel. When the batch capacity is reached
 * it's flushed to the GPU.
 *
 * @param scene entt::registry containing entities and components.
 */
void Renderer::render(Scene& scene) {
	begin();
	
	auto view = scene.view<RenderComponent, TransformComponent>(); //!< Get all entities with a RenderComponent
	view.each([&](auto& renderComp, auto& transformComp) {

		/** Push vertices to CPU-side vertex buffer to be accessed by SSBO */
		glm::vec2 vert;
		vert.x = renderComp.pos.x - renderComp.halfExtent.x;
		vert.y = renderComp.pos.y - renderComp.halfExtent.y;
		m_cpuVertBuffer.push_back({
			.colour = renderComp.colour, //!< Vertex colour
			.position = glm::vec2(transformComp.transform * glm::vec3(vert, 1.f)) + renderComp.pos,
			.uv = { renderComp.u.x, renderComp.v.y },
		});

		vert.x = renderComp.pos.x - renderComp.halfExtent.x;
		vert.y = renderComp.pos.y + renderComp.halfExtent.y;
		m_cpuVertBuffer.push_back({
			.colour = renderComp.colour, //!< Vertex colour
			.position = glm::vec2(transformComp.transform * glm::vec3(vert, 1.f)) + renderComp.pos,
			.uv = { renderComp.u.x, renderComp.v.x },
		});
		
		vert.x = renderComp.pos.x + renderComp.halfExtent.x;
		vert.y = renderComp.pos.y - renderComp.halfExtent.y;
		m_cpuVertBuffer.push_back({
			.colour = renderComp.colour, //!< Vertex colour
			.position = glm::vec2(transformComp.transform * glm::vec3(vert, 1.f)) + renderComp.pos,
			.uv = { renderComp.u.y, renderComp.v.y },
		});

		vert.x = renderComp.pos.x + renderComp.halfExtent.x;
		vert.y = renderComp.pos.y + renderComp.halfExtent.y;
		m_cpuVertBuffer.push_back({
			.colour = renderComp.colour, //!< Vertex colour
			.position = glm::vec2(transformComp.transform * glm::vec3(vert, 1.f)) + renderComp.pos,
			.uv = { renderComp.u.y, renderComp.v.x },
		});

		m_textureHandles.push_back(renderComp.textureHandle); //!< Store texture handle for later use
		
		if (m_cpuVertBuffer.size() >= s_vertexCapacity || m_textureHandles.size() >= s_textureCapacity) {
			flush(); //!< Flush the batch if vertex or texture capacity is exceeded
		}
	});

	end();
}

/**
 * @brief Begin a new frame: clear CPU buffers and set per-frame GL state.
 *
 * Responsibilities:
 * - Clear CPU-side buffers used for batching.
 * - Clear the GL color buffer and set the viewport.
 * - Upload current camera view and projection matrices into the shader program.
 *
 * @note The shader program must be bound (glUseProgram) before calling this.
 */
void Renderer::begin() {

	m_cpuVertBuffer.clear(); //!< Clear CPU-side vertex buffer for new frame
	m_textureHandles.clear(); //!< Clear texture handles for new frame

	glClear(GL_COLOR_BUFFER_BIT); //!< Clear the screen to prepare for rendering
	glViewport(0, 0, 1024, 800); //!< Set viewport to match window dimensions
}

/**
 * @brief Upload current batch to GPU SSBOs and issue draw call(s).
 *
 * Operation:
 * - Updates the vertex SSBO and the texture-handle SSBO with the CPU-side buffers.
 * - Binds the VAO and issues an instanced draw where each instance corresponds to one quad.
 *
 * @note The vertex layout and shader must interpret the SSBO contents correctly.
 */
void Renderer::flush() {

	glNamedBufferSubData(m_vertSsboID, 0, sizeof(Vertex) * m_cpuVertBuffer.size(), (const void*)m_cpuVertBuffer.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, m_vertSsboID); //!< Bind VBO to SSBO binding point 0 for shader access

	glNamedBufferSubData(m_texSsboID, 0, sizeof(uint64_t) * m_textureHandles.size(), (const void*)m_textureHandles.data());
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, m_texSsboID); //!< Bind VBO to SSBO binding point 1 for shader access

	/** Bind VAO */
	glBindVertexArray(m_vaoID);

	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, m_cpuVertBuffer.size() / 4); //!< Draw the quads using instanced rendering
	//glDrawArraysInstanced(GL_POINTS, 0, 4, m_cpuVertBuffer.size() / 4); //!< Draw the quads using instanced rendering
}

/** @brief Finish the frame by flushing any remaining buffered data. */
void Renderer::end() {
	flush(); //!< Ensure any remaining batches are flushed after rendering
}

/**
 * @brief Compile shader source from a file into a GL shader object.
 *
 * Reads the file at `src`, compiles it as `shaderType` and returns the created
 * shader object id. On compilation failure the shader object is deleted and
 * an info log is printed to stdout.
 *
 * @param shaderObj Existing shader object id (ignored; overwritten with new object).
 * @param src Filesystem path pointing to shader source.
 * @param shaderType GL shader type constant (e.g. GL_VERTEX_SHADER).
 * @return GLuint compiled shader object id, or 0 on failure.
 * @throws std::runtime_error if shader creation fails (glCreateShader returns 0).
 */
GLuint Renderer::CompileShader(GLuint shaderObj, const std::filesystem::path& src, GLuint shaderType)
{
	std::fstream vsHandle(src.c_str(), std::ios::in);

	std::stringstream sStream;
	sStream << vsHandle.rdbuf();
	vsHandle.close();

	shaderObj = glCreateShader(shaderType);
	if (shaderObj == 0) {
		throw std::runtime_error("failed to create shader");
	}
	std::string tmp = sStream.str();
	const GLchar* source = tmp.c_str();

	glShaderSource(shaderObj, 1, &source, 0);
	glCompileShader(shaderObj);

	GLint isCompiled = 0;
	glGetShaderiv(shaderObj, GL_COMPILE_STATUS, &isCompiled);
	if (isCompiled == GL_FALSE)
	{
		GLint maxLength = 0;
		glGetShaderiv(shaderObj, GL_INFO_LOG_LENGTH, &maxLength);

		std::vector<GLchar> infoLog(maxLength);
		glGetShaderInfoLog(shaderObj, maxLength, &maxLength, &infoLog[0]);
		std::cout << "Shader compile error: {0}" << std::string(infoLog.begin(), infoLog.end()) << std::endl;

		glDeleteShader(shaderObj);
		shaderObj = 0;
	}

	return shaderObj;
}

/**
 * @brief Load an image file and create an OpenGL 2D texture.
 *
 * Uses stb_image to load pixel data and upload it into a GL texture with
 * linear filtering and mipmap generation.
 *
 * @param texDesc Descriptor containing path and expected format hints.
 * @return GLuint OpenGL texture id (non-zero) on success.
 * @throws std::runtime_error when image loading fails.
 */
GLuint Renderer::loadTexture(TextureDescription texDesc) {
	GLuint texture = 0;
	if (!std::filesystem::exists(texDesc.src)) {
		std::cout << "Failed to locate texture from provided path: " << texDesc.src << std::endl;
		return texture;
	}
	glGenTextures(1, &texture);
	glBindTexture(GL_TEXTURE_2D, texture);
	/** Setting texture parameters */
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/** Load data of texture */
	unsigned char* data = stbi_load(texDesc.src, &texDesc.width, &texDesc.height, &texDesc.channels, 4);
	if (data) {
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, texDesc.width, texDesc.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	}
	else {
		throw std::runtime_error("Failed to load texture");
	}
	stbi_image_free(data);
	return texture;
}

/**
 * @brief Acquire a bindless texture handle and make it resident.
 *
 * Obtains a handle suitable for passing to shaders using ARB_bindless_texture,
 * makes it resident and returns it.
 *
 * @param texture OpenGL texture id previously created by glGenTextures/glTexImage2D.
 * @return GLuint64 handle that can be passed to shaders (0 on failure).
 *
 * @note Callers should ensure the GL implementation supports ARB_bindless_texture
 *       before using this function. Handles are tracked by the renderer and
 *       (attempted to be) made non-resident in the destructor.
 */
GLuint64 Renderer::getHandle(GLuint texture) {
	GLuint64 handle = 0;
	handle = glGetTextureHandleARB(texture); //!< Get texture handle for bindless textures
	glMakeTextureHandleResidentARB(handle); //!< Make texture handle resident for shader access
	return handle;
}