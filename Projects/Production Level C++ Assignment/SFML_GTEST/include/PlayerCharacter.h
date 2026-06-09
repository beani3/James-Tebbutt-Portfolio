#include "DynamicObject.h"
#include "AnimationManager.h"
#include <string>
#pragma once

class PlayerCharacter : public DynamicObject {
public:
	PlayerCharacter() = default; // Default constructor for player character.
	PlayerCharacter(float f_speed, float f_health, sf::Vector2f v2_pos, float f_angle, sf::Vector2f v2_sf);

	/* Getters and Setters for the player character
	 * values.
	 */
	float& getSpeed() { return f_speed; }
	void setSpeed(float speed) { f_speed = speed; }
	float& getHealth() { return f_health; }
	void setHealth(float health) { f_health = health; }
	sf::Vector2f getPos() { return s_sprite.getPosition(); }
	void setPos(sf::Vector2f pos) { s_sprite.setPosition(pos); }
	sf::Vector2f getScale() { return v2_sf; }
	void setScale(sf::Vector2f sf) { s_sprite.setScale(sf); }
	sf::Sprite& getSprite() { return s_sprite; }
	AnimationManager& getAnimManager() { return m_animationManager; }
	void setSprite(sf::Sprite sprite) { s_sprite = sprite; }
	sf::FloatRect getCollider() { return fr_collider; }
	void setCollider(sf::FloatRect collider) { fr_collider = collider; }

	/* Declaring Functions in PlayerCharacter.cpp
	 */
	void takeDamage(float f_amount);
	void move(float dt);
	void setColliderPos(float x, float y);
	void draw(sf::RenderWindow& window);
private:
	float f_speed;
	float f_health;
	sf::Vector2f v2_pos;
	sf::Vector2f v2_sf;
	sf::Sprite s_sprite;
	AnimationManager m_animationManager;
};