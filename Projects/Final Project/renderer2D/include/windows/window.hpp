#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <memory>
#include <glm/glm.hpp>

/**
 * @file window.hpp
 * @brief Window abstraction wrapper around GLFW for the renderer.
 *
 * Provides platform window creation, per-frame updates and simple timing
 * helpers used by the renderer and demo application.
 */

 /**
  * @brief Configuration properties used when creating a window.
  *
  * Populate an instance of this struct and pass it to `Window::openWindow()`
  * to control title, size and fullscreen / resizability behavior.
  */
struct WindowProperties {
	// Window title string (null-terminated C string).
	const char* title{ "" };
	// Window size in pixels (width, height).
	glm::ivec2 size{ 800, 600 };
	// Whether the window can be resized by the user.
	bool isResizeable{ false };
	// Whether the window should be created fullscreen.
	bool isFullscreen{ false };

	WindowProperties() = default;

	/**
	 * @brief Construct a WindowProperties instance with explicit values.
	 * @param newTitle Null-terminated C string for the window title.
	 * @param s Initial window size as glm::ivec2(width, height).
	 * @param resizeable If true, the window will be resizable.
	 * @param fullscreen If true, the window will be created fullscreen.
	 */
	WindowProperties(const char* newTitle, glm::ivec2 s, bool resizeable, bool fullscreen) :
		title(newTitle),
		size(s),
		isResizeable(resizeable),
		isFullscreen(fullscreen)
	{}
};

/**
 * @brief Unique pointer deleter type for managing GLFWwindow lifetime.
 *
 * This type can be used with `std::unique_ptr<GLFWwindow, GLFWWinDeleter>`
 * to ensure `glfwDestroyWindow` is called when the wrapper is destroyed.
 */
using GLFWWinDeleter = decltype(
	[](GLFWwindow* window) {
		glfwDestroyWindow(window);
	});

/**
 * @brief Lightweight platform window wrapper.
 *
 * Encapsulates GLFW window creation, event polling and basic frame timing.
 * The class owns the GLFWwindow and provides a simple lifecycle:
 * initialize -> open -> per-frame onUpdate -> query shouldClose.
 */
class Window {
public:
	Window() = default;

	/**
	 * @brief Initialize the underlying window subsystem.
	 *
	 * This typically initializes GLFW, configures context hints, and performs
	 * any global one-time setup required before creating windows.
	 *
	 * @note Call this before `openWindow()`.
	 */
	void initWindow();

	/**
	 * @brief Create and open a window with the provided properties.
	 * @param properties WindowProperties instance describing title, size and mode.
	 *
	 * This will create the GLFW window and set up the OpenGL context and any
	 * required callbacks. The function may modify internal state used by
	 * `onUpdate()` and `shouldClose()`.
	 */
	void openWindow(WindowProperties& properties);

	/**
	 * @brief Perform a single frame update for the window.
	 *
	 * Typical responsibilities include polling events, swapping buffers,
	 * and updating per-frame timing. Call this once per frame from the main loop.
	 */
	void onUpdate();

	/**
	 * @brief Query whether the window has requested to close.
	 * @return True if the window should close, false otherwise.
	 */
	inline bool shouldClose() { return m_shouldClose; }
	GLFWwindow* getGLFWwindow() const { return m_window.get(); }



private:
	/** Managed GLFW window handle.Uses custom deleter to call glfwDestroyWindow. */
	std::unique_ptr<GLFWwindow, GLFWWinDeleter> m_window;
	/** Current frame time in seconds.*/
	float m_currentFrame = 0.0f;
	/** Time elapsed between current and last frame in seconds. */
	float m_deltaTime = 0.0f;
	/** Last frame time in seconds. */
	float m_lastFrame = 0.0f;

	/**
	 * @brief Internal per-frame tick to update timing values.
	 *
	 * Updates `m_currentFrame`, `m_deltaTime` and `m_lastFrame`. Invoked by
	 * `onUpdate()` to maintain accurate frame timing for animations / updates.
	 */
	void tick();
	/** Internal flag indicating whether the window has requested closure. */
	bool m_shouldClose{ false };
};