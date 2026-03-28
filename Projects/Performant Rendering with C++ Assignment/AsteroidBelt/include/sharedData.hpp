/** \file sharedData.hpp */
#pragma once
#include <glm/glm.hpp>

/** \struct SharedData 
*	\brief Struct containing data shared between layers
*/

enum class GameState { intro, running, gameOver };

struct SharedData {
	std::vector<glm::vec3> closeAsteroids; // Asteroids to be drawn in the UI (x,y) dist
	std::vector<glm::vec3> closeTargets; // Targets to be drawn in the UI (x,y) dist
	GameState state{ GameState::intro };
	float speed{ 0.f };
};
