#pragma once
#include "core/application.hpp"
#include "finalProject.hpp"
#include <filesystem>
#include <fstream>

/**
 * @file main.cpp
 * @brief Program entry point for the application.
 *
 * Creates a `Application` instance, initializes application subsystems and
 * enters the application's main loop.
 */

 /**
  * @brief Application entry point.
  *
  * Constructs the `Application` object, calls `init()` to prepare required
  * resources (window, renderer, etc.) and then calls `run()` to execute
  * the main loop until the application exits.
  *
  * @return Exit code (0 indicates successful execution).
  */
int main() {

	Application app;
	app.init();
	app.run();
	return 0;
}