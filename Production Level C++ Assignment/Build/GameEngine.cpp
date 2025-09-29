#include "GameEngine.h"
#include "StaticObject.h"
#include "EnemyBuilder.h"
#include "SplashScreen.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>

GameEngine::GameEngine(sf::RenderWindow& window) :
    m_window(window),
    m_playerCharacter(100, 100, sf::Vector2f((window.getSize().x / 2) + 100, window.getSize().y / 2), 0, sf::Vector2f(1, 1))
{
    m_gameStates = GameStates::startGame;
    m_enemy = m_enemyBuilder
        .buildHealth(100)
        .buildSpeed(100)
        .buildPos(sf::Vector2f((window.getSize().x / 2) - 100, window.getSize().y / 2))
        .buildScale(sf::Vector2f(1, 1))
        .buildSprite("../Assets/TinySwords/Factions/Goblins/Troops/TNT/Blue/TNT_Blue-Idle.png", 0, 0, sf::Vector2f(192, 192))
        .buildCollider(m_enemyBuilder.getSprite().getPosition(), 64, 64)
        .build();

    m_font.loadFromFile("../Assets/Fonts/calibri.ttf");
    m_text.setColor(sf::Color::White);
}

/******************************
 * Testing Constructor 
 ******************************/
GameEngine::GameEngine(sf::RenderWindow& window, PlayerCharacter& player, std::shared_ptr<EnemyNPC> enemy) :
    m_window(window)
{
    this->m_playerCharacter = player;
    this->m_enemy = enemy;
}

GameEngine::GameEngine(sf::RenderWindow& window, PlayerCharacter& player, Tilemap& tilemap) :
    m_window(window)
{
    this->m_playerCharacter = player;
    this->m_tilemap = tilemap;
}

GameEngine::GameEngine(sf::RenderWindow& window, PlayerCharacter& player) :
    m_window(window)
{
    this->m_playerCharacter = player;
}

/******************************
 * Input Management
 ******************************/
void GameEngine::handleMovement(float dt) {
    // Player Movement
    // Sprint
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::LShift)) {
        m_playerCharacter.setSpeed(150);
    }
    else {
        m_playerCharacter.setSpeed(100);
    }

    // Move player up
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Up))) {
        m_playerCharacter.move(dt);
        b_playerMoved = true;
    }

    // Move player down
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Down))) {
        m_playerCharacter.move(dt);
        b_playerMoved = true;
    }

    // Move player left
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Left))) {
        m_playerCharacter.move(dt);
        m_playerCharacter.setScale(sf::Vector2f(-1, 1));
        b_playerMoved = true;
    }

    // Move player right
    if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || (sf::Keyboard::isKeyPressed(sf::Keyboard::Right))) {
        m_playerCharacter.move(dt);
        m_playerCharacter.setScale(sf::Vector2f(1, 1));
        b_playerMoved = true;
    }

    // Moving collider relative to player movement.
    m_playerCharacter.setColliderPos(m_playerCharacter.getPos().x, m_playerCharacter.getPos().y);

    // Enemy Movement
    sf::Vector2f v2_moveDir;
    // Get the Move Direction
    v2_moveDir = m_playerCharacter.getPos() - m_enemy->getSprite().getPosition();
    // Calculate Magnitude 
    float f_magnitude = std::sqrt((v2_moveDir.x * v2_moveDir.x) + (v2_moveDir.y * v2_moveDir.y));
    sf::Vector2f v2_unitDir;
    // Calculate Unit Normal of Move Direction
    v2_unitDir = sf::Vector2f(v2_moveDir.x / f_magnitude, v2_moveDir.y / f_magnitude);

    // Move enemy
    m_enemy->getSprite().move(v2_unitDir * (m_enemy->getSpeed() * dt));
    m_enemy->setPos(m_enemy->getSprite().getPosition());
    m_enemy->setColliderPos(m_enemy->getPos().x, m_enemy->getPos().y);

    if (v2_unitDir.x < 0) {
        m_enemy->setScale(sf::Vector2f(-1, 1));
    } else {
        m_enemy->setScale(sf::Vector2f(1, 1));
    }
}

/******************************
 * Collision Detection
 ******************************/
void GameEngine::handleCollisions(float dt) { 
    f_time += dt;

    // Player Enemy Collision
    if (m_playerCharacter.getCollider().intersects(m_enemy->getCollider())) {
        if (f_time - f_timeSinceLastHit > f_damageCooldown) {
            m_playerCharacter.takeDamage(25);
            f_timeSinceLastHit = f_time;
            c_clock.restart();
        }

        b_enemyCol = true;
    }

    // Player Tilemap Collision
    // Top
    if (m_playerCharacter.getCollider().top < m_tilemap.getCollider().top
        && m_playerCharacter.getCollider().left + m_playerCharacter.getCollider().width > m_tilemap.getCollider().left
        && m_playerCharacter.getCollider().left < m_tilemap.getCollider().left + m_tilemap.getCollider().width) {
        m_playerCharacter.getSprite().move(0, 1);
        b_tilemapCol = true;
    }
    // Bottom
    if (m_playerCharacter.getCollider().top + m_playerCharacter.getCollider().height > m_tilemap.getCollider().top + m_tilemap.getCollider().height
        && m_playerCharacter.getCollider().left + m_playerCharacter.getCollider().width > m_tilemap.getCollider().left
        && m_playerCharacter.getCollider().left < m_tilemap.getCollider().left + m_tilemap.getCollider().width) {
        m_playerCharacter.getSprite().move(0, -1);
        b_tilemapCol = true;
    }
    // Left
    if (m_playerCharacter.getCollider().left < m_tilemap.getCollider().left
        && m_playerCharacter.getCollider().top + m_playerCharacter.getCollider().height > m_tilemap.getCollider().top
        && m_playerCharacter.getCollider().top < m_tilemap.getCollider().top + m_tilemap.getCollider().height) {
        m_playerCharacter.getSprite().move(1, 0);
        b_tilemapCol = true;
    }
    // Right
    if (m_playerCharacter.getCollider().left + m_playerCharacter.getCollider().width > m_tilemap.getCollider().left + m_tilemap.getCollider().width
        && m_playerCharacter.getCollider().top + m_playerCharacter.getCollider().height > m_tilemap.getCollider().top
        && m_playerCharacter.getCollider().top < m_tilemap.getCollider().top + m_tilemap.getCollider().height) {
        m_playerCharacter.getSprite().move(-1, 0);
        b_tilemapCol = true;
    }

    // Enemy Tilemap Collision 
    // Top
    if (m_enemy->getCollider().top < m_tilemap.getCollider().top
        && m_enemy->getCollider().left + m_enemy->getCollider().width > m_tilemap.getCollider().left
        && m_enemy->getCollider().left < m_tilemap.getCollider().left + m_tilemap.getCollider().width) {
        m_enemy->getSprite().move(0, 1);
        b_tilemapCol = true;
    }
    // Bottom
    if (m_enemy->getCollider().top + m_enemy->getCollider().height > m_tilemap.getCollider().top + m_tilemap.getCollider().height
        && m_enemy->getCollider().left + m_enemy->getCollider().width > m_tilemap.getCollider().left
        && m_enemy->getCollider().left < m_tilemap.getCollider().left + m_tilemap.getCollider().width) {
        m_enemy->getSprite().move(0, -1);
        b_tilemapCol = true;
    }
    // Left
    if (m_enemy->getCollider().left < m_tilemap.getCollider().left
        && m_enemy->getCollider().top + m_enemy->getCollider().height > m_tilemap.getCollider().top
        && m_enemy->getCollider().top < m_tilemap.getCollider().top + m_tilemap.getCollider().height) {
        m_enemy->getSprite().move(1, 0);
        b_tilemapCol = true;
    }
    // Right
    if (m_enemy->getCollider().left + m_enemy->getCollider().width > m_tilemap.getCollider().left + m_tilemap.getCollider().width
        && m_enemy->getCollider().top + m_enemy->getCollider().height > m_tilemap.getCollider().top
        && m_enemy->getCollider().top < m_tilemap.getCollider().top + m_tilemap.getCollider().height) {
        m_enemy->getSprite().move(-1, 0);
        b_tilemapCol = true;
    }
}

/***********************
 * Update UI
 ***********************/
void GameEngine::update() {
    switch (m_gameStates) {
    case GameEngine::startGame:
        m_text = sf::Text("Press Enter to Start", m_font, 50);
        m_text.setOrigin(m_text.getGlobalBounds().width / 2, m_text.getGlobalBounds().height / 2);
        m_text.setPosition(m_window.getSize().x / 2, m_window.getSize().y / 2);

        m_window.clear();
        m_window.draw(m_text);
        m_window.display();
        break;
    case GameEngine::playing:
        std::cout << "Health: " << m_playerCharacter.getHealth() << std::endl;
        draw();

        if (m_playerCharacter.getHealth() <= 0) {
            m_gameStates = GameStates::gameOver;
        }
        break;
    case GameEngine::gameOver:
        m_text = sf::Text("You have Died! \nPress Enter to Start New Game", m_font, 50);
        m_text.setOrigin(m_text.getGlobalBounds().width / 2, m_text.getGlobalBounds().height / 2);
        m_text.setPosition(m_window.getSize().x / 2, m_window.getSize().y / 2);

        m_window.clear();
        m_window.draw(m_text);
        m_window.display();

        if (sf::Keyboard::isKeyPressed(sf::Keyboard::Enter)) {
            m_playerCharacter.setHealth(100);
            m_playerCharacter.setPos(sf::Vector2f((m_window.getSize().x / 2) + 100, m_window.getSize().y / 2));
            m_enemy->setPos(sf::Vector2f((m_window.getSize().x / 2) - 100, m_window.getSize().y / 2));
            m_gameStates = GameStates::playing;
        }
        break;
    default:
        break;
    }
}

void GameEngine::draw() {
    m_window.clear();
    m_tilemap.drawTilemap(m_window);
    m_enemy->draw(m_window);
    m_playerCharacter.draw(m_window);
    m_window.display();
}

/***********************
 * Main Game Loop
 ***********************/
void GameEngine::run() {
    while (m_window.isOpen()) {  
        /* Stops inputs causing the player to zoom in the 
         * inputted direction
         */
        float dt;
        dt = c_clock.restart().asSeconds();

        // Poll for events in the window
        sf::Event event;
        while (m_window.pollEvent(event)) {
            // Close the window if the user clicks on the close button
            if (event.type == sf::Event::Closed) {                            
                m_window.close();
            }
            if (event.type == sf::Event::KeyPressed && event.key.code == sf::Keyboard::Enter) {
                m_gameStates = GameStates::playing;
            }
            if (event.type == sf::Event::Resized) {
                m_window.setSize(sf::Vector2u(1920, 1080));
            }
        }                                                                  

        /* Checks if the game is playing before enabling 
         * movement.
         */
        if (m_gameStates == GameStates::playing) {
            // Move Player
            handleMovement(dt);
            handleCollisions(dt);
        }

        // Update UI 
        update();
    }
}
