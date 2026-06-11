#include "core/application.hpp"
#include <core/input.hpp>
#include <iostream>

/**
 * @file application.cpp
 * @brief Application lifecycle implementation.
 *
 * The Application wrapper is responsible for bootstrapping subsystems (Window,
 * Input, game) and driving the main loop. It performs minimal per-frame
 * coordination: update, render and check for global exit input.
 */

/**
 * @brief Initialize the application and subsystems.
 *
 * Actions:
 * - Configure WindowProperties (title, size, resizable).
 * - Initialize and open the platform window.
 * - Wire InputSystem to the window instance.
 * - Instantiate the FinalProject game object.
 *
 * After successful initialization, m_isRunning is set to true so run() will execute.
 */
void Application::init() {
	// Configure window properties
	WindowProperties winProp;
	winProp.title = "Renderer Demonstration : The Game";
	winProp.size = glm::ivec2(1024, 800);
	winProp.isResizeable = false;

	// Initialize window subsystem and open the window with configured properties
	m_window.initWindow();
	m_window.openWindow(winProp);
	InputSystem::get().m_rawWinPtr = &m_window;
	m_finalProject = new FinalProject();
	// Mark the application as running
	m_isRunning = true;
}

/**
 * @brief Run the application's main loop.
 *
 * Loop behaviour:
 * - While running, poll/dispatch window events and update the game.
 * - Call FinalProject::onRender() and FinalProject::onUpdate() each frame.
 * - When the window is requested to close or Escape is pressed, exit the loop.
 *
 * On exit, terminate GLFW to clean up the platform resources.
 */
void Application::run() {

	while (m_isRunning) {
		// Exit when the window requests close
		m_isRunning = !m_window.shouldClose();
		// Perform per-frame update (event polling, swap buffers, etc.)

		m_finalProject->onRender();
		m_finalProject->onUpdate();
		m_window.onUpdate();
		onKeyPressed();
	}
	glfwTerminate();
}

/**
 * @brief Global key handling (Escape to quit).
 *
 * Called from the main loop to check for application-level key commands.
 * This is intentionally minimal and forwards per-frame input to the game.
 */
void Application::onKeyPressed() {
	if (InputSystem::get().isKeyPressed(GLFW_KEY_ESCAPE)) {
		std::cout << "Exiting program" << std::endl;
		m_isRunning = false;
	}
}
