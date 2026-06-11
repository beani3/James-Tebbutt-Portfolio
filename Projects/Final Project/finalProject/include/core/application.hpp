#pragma once  
#include "renderer2D.hpp"
#include "finalProject.hpp"

/**
 * @file application.hpp
 * @brief Application entry wrapper.
 *
 * Contains the `Application` class which is responsible for initializing
 * subsystems (window, renderer, etc.) and driving the main loop.
 */

/**
 * @brief Simple application class.
 *
 * The `Application` class owns the platform/window wrapper and exposes
 * lifecycle methods to initialize resources and run the application's
 * main loop. It also links the InputSystem to the created window instance.
 */
class Application {
public:
	/**
	 * @brief Initialize the application.
	 *
	 * This sets up the window, the InputSystem and constructs the FinalProject
	 * instance. After successful init, run() may be called to start the main loop.
	 */
	void init();

	/**
	 * @brief Run the application's main loop.
	 *
	 * This will poll events, update state and render frames until the
	 * window indicates it should close or the user requests exit.
	 */
	void run();

	/**
	 * @brief Handle global key presses (application-level).
	 *
	 * Checks for keys such as Escape and performs application-level actions.
	 */
	void onKeyPressed();
private:
	/**
	 * @brief Indicates whether the main loop is currently running.
	 *
	 * Set to `true` while `run()` is executing; used to control loop exit
	 * and orderly shutdown of resources.
	 */
	bool m_isRunning{ false };
	/**
	 * @brief Platform/window abstraction.
	 *
	 * Provides window creation, event polling, frame timing and close
	 * state. See `Window` for available operations such as `initWindow()`,
	 * `openWindow()`, `onUpdate()` and `shouldClose()`.
	 */
	Window m_window;
	FinalProject* m_finalProject{ nullptr }; //!< Owned pointer to the game instance
};