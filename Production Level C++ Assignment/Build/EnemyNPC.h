#include "DynamicObject.h"
#include <string>
#pragma once

class EnemyNPC : public DynamicObject {
public:
	EnemyNPC() = default;
	EnemyNPC(float f_health, float f_speed, sf::Vector2f v2_pos, sf::Vector2f v2_sf, sf::Sprite s_sprite, sf::FloatRect fr_collider) :
		f_health(f_health),
		f_speed(f_speed),
		v2_pos(v2_pos),
		v2_sf(v2_sf),
		s_sprite(s_sprite),
		fr_collider(fr_collider) {}

	/* Getters and Setters for the Enemy NPC
	 * values.
	 */
	float& getSpeed() { return f_speed; }
	void setSpeed(float speed) { 
		f_speed = speed; 
	}
	float& getHealth() { return f_health; }
	void setHealth(float health) {
		f_health = health; 
	}
	sf::Vector2f getPos() { return v2_pos; }
	void setPos(sf::Vector2f pos) {
		v2_pos = pos; 
	}
	sf::Vector2f getScale() { return v2_sf; }
	void setScale(sf::Vector2f sf) { 
		s_sprite.setScale(sf); 
	}
	sf::Sprite& getSprite() { return s_sprite; }
	void setSprite(sf::Sprite sprite) { 
		s_sprite = sprite; 
	}
	sf::FloatRect getCollider() { return fr_collider; }
	void setCollider(sf::FloatRect collider) { 
		fr_collider = collider; 
	}

	/* Declaring Functions in EnemyNPC.cpp
	 */
	void draw(sf::RenderWindow& window);
	void setColliderPos(float x, float y);
private:
	float f_speed;
	float f_health;
	sf::Vector2f v2_pos;
	sf::Vector2f v2_sf;
	sf::Sprite s_sprite;
	sf::Texture t_texture;
	sf::FloatRect fr_collider;
};