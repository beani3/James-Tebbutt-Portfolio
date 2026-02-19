#include "PlayerCharacter.h"
#include "EnemyNPC.h"
#include "Tilemap.h"
#include "SplashScreen.h"
#include "EnemyBuilder.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>
#include <sstream>
#include <fstream>
#include <mutex>
#include <thread>
#include <chrono>
#pragma once

class GameEngine {
public:
	GameEngine(sf::RenderWindow& window);
	// Testing Constructors
	GameEngine(sf::RenderWindow& window, PlayerCharacter& player, std::shared_ptr<EnemyNPC> enemy);
	GameEngine(sf::RenderWindow& window, PlayerCharacter& player, Tilemap& tilemap);
	GameEngine(sf::RenderWindow& window, PlayerCharacter& player);
	void handleMovement(float dt);
	void handleCollisions(float dt);
	void draw();
	void update();
	void run();

	/* Getters for testing
	 */
	bool getEnemyCol() { return b_enemyCol; }
	bool getTilemapCol() { return b_tilemapCol; }
	bool getMoved() { return b_playerMoved; }
private:
	sf::Clock c_clock;
	float f_time = 0;
	sf::RenderWindow& m_window;
	PlayerCharacter m_playerCharacter;
	Tilemap m_tilemap;
	AnimationManager m_animManager;
	EnemyBuilder m_enemyBuilder;
	int i_enemies;
	
	/* Variables for collision 
	 */
	float f_elapsedTime;
	float f_timeSinceLastHit;
	float f_damageCooldown = 3.0f;
	// Testing Variables
	bool b_enemyCol = false;
	bool b_tilemapCol = false;
	bool b_playerMoved = false;

	/* Variables and functions for the UI and game states
	 */
	enum GameStates { startGame, playing, gameOver };
	GameStates m_gameStates;
	sf::Font m_font;
	sf::Text m_text;
	/* Multi-threading and splashscreen variables
	 */
	SplashScreen m_splashScreen;
protected:
	std::shared_ptr<EnemyNPC> m_enemy;
};