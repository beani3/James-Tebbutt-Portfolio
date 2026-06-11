#pragma once
#include "windows/window.hpp"
#include "rendering/renderer.hpp"
#include "core/input.hpp"
#include <iostream>

/**
 * @file window.cpp
 * @brief Implementation of the Window wrapper around GLFW.
 *
 * Implements initialization, creation, per-frame update and timing logic
 * for the `Window` class declared in `window.hpp`.
 */

 /**
  * @brief Initialize the underlying window subsystem (GLFW).
  *
  * Configures GLFW and sets the OpenGL context hints used when creating
  * the window. This must be called before `openWindow()`.
  */
void Window::initWindow() {

	if (!glfwInit()) {
		throw std::runtime_error("Failed to initialise GLFW");
	}

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
	glfwWindowHint(GLFW_RESIZABLE, GL_FALSE);
}

/**
 * @brief Create and open a GLFW window using the provided properties.
 *
 * Allocates the GLFW window and stores it in the managed unique_ptr with
 * a custom deleter. Sets the OpenGL context current on the created window,
 * registers the `Window` instance as the GLFW user pointer and installs a
 * close-callback which sets the internal close flag.
 *
 * @param properties WindowProperties describing title, size and mode.
 */
void Window::openWindow(WindowProperties& properties) {
	m_window = std::unique_ptr<GLFWwindow, GLFWWinDeleter>(glfwCreateWindow(properties.size.x, properties.size.y, properties.title, nullptr, nullptr));

	if (m_window == nullptr) {
		glfwTerminate();
		throw std::runtime_error("Failed to create window");
	}
	glfwMakeContextCurrent(m_window.get());

	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
		throw std::runtime_error("Failed to initialize GLAD");

	glEnable(GL_DEBUG_OUTPUT);
	glDebugMessageCallback(
		[](
			GLenum source,
			GLenum type,
			GLuint id,
			GLenum severity,
			GLsizei length,
			const GLchar* msg,
			const void* userParam)
		{
			if (severity == GL_DEBUG_SEVERITY_LOW || severity == GL_DEBUG_SEVERITY_MEDIUM || severity == GL_DEBUG_SEVERITY_HIGH) {
				std::cout << "GL ERROR\n" << msg << std::endl;
				std::cout << "src " << source << std::endl;
				std::cout << "id " << id << std::endl;
			}
		}
		, nullptr
	);

	glfwSwapInterval(0);

	std::cout << "Version " << glGetString(GL_VERSION) << std::endl;

	glfwSetWindowUserPointer(m_window.get(), static_cast<void*>(this));

	glfwSetWindowCloseCallback(m_window.get(),
		[](GLFWwindow* window)
		{
			Window* win = static_cast<Window*>(glfwGetWindowUserPointer(window));
			win->m_shouldClose = true;
		}
	);

	glfwSetKeyCallback(m_window.get(),
		[](GLFWwindow* window, int key, int scancode, int action, int mods) {
			InputSystem::get().m_keyStates[key] = action;
		}
	);
}

/**
 * @brief Perform a single frame update for the window.
 *
 * Polls pending events, swaps the front and back buffers for the OpenGL
 * context and updates per-frame timing via `tick()`.
 */
void Window::onUpdate() {
	float timestep = m_deltaTime;
	glfwPollEvents();
	glfwSwapBuffers(m_window.get());
	tick();
}

/**
 * @brief Update internal timing values for the current frame.
 *
 * Reads the global GLFW time and computes `m_deltaTime` as the difference
 * from the previous frame time. Updates `m_lastFrame` for the next tick.
 */
void Window::tick() {
	m_currentFrame = glfwGetTime();
	m_deltaTime = m_currentFrame - m_lastFrame;
	m_lastFrame = m_currentFrame;
}