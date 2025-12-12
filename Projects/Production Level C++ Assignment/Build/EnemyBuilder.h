#include "NPCBuilder.h"
#include <iostream>
#pragma once
class EnemyBuilder : public NPCBuilder {
public:
	~EnemyBuilder() override = default;
	NPCBuilder& buildHealth(float health) override {
		f_health = health;
		return *this;
	}
	NPCBuilder& buildSpeed(float speed) override {
		f_speed = speed;
		return *this;
	}
	NPCBuilder& buildPos(sf::Vector2f pos) override {
		v2_pos = pos;
		return *this;
	}
	NPCBuilder& buildScale(sf::Vector2f sf) override {
		v2_sf = sf;
		return *this;
	}
	NPCBuilder& buildSprite(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size) override {
		if (!t_texture.loadFromFile(s_dir)) {
			std::cout << "Failed to load texture" << std::endl;
			b_loadedSprite = false;
		}

		ir_sourceSprite = sf::IntRect(i_left, i_top, v2_size.x, v2_size.y);
		s_sprite = sf::Sprite(t_texture, ir_sourceSprite);
		s_sprite.setOrigin(s_sprite.getTextureRect().width / 2, s_sprite.getTextureRect().height / 2);
		s_sprite.setPosition(v2_pos);
		b_loadedSprite = true;
		return *this;
	}

	NPCBuilder& buildCollider(sf::Vector2f v2_position, float f_width, float f_height) override {
		fr_collider = sf::FloatRect(v2_position.x - (f_width / 2), v2_position.y - (f_height / 2), f_width, f_height);
		b_createdCollider = true;
		return *this;
	}

	/* Builds an enemy with the variables set using the 
	 * functions above 
	 */ 
	std::shared_ptr<EnemyNPC> build() override {
		m_enemyNPC = std::make_shared<EnemyNPC>(f_health, f_speed, v2_pos, v2_sf, s_sprite, fr_collider);
		return m_enemyNPC;
	}

	// Getters for testing variables
	float getHealth() { return f_health; }
	float getSpeed() { return f_speed; }
	sf::Vector2f getPos() { return v2_pos; }
	sf::Vector2f getScale() { return v2_sf; }
	sf::Sprite getSprite() { return s_sprite; }
	sf::FloatRect getCollider() { return fr_collider; }
	bool getLoadSprite() { return b_loadedSprite; }
	bool getCreateCollider() { return b_createdCollider; }
private:
	float f_health;
	float f_speed;
	sf::Vector2f v2_pos;
	sf::Vector2f v2_sf;
	sf::Sprite s_sprite;
	sf::Texture t_texture;
	sf::IntRect ir_sourceSprite;
	sf::FloatRect fr_collider;
	// Variables for testing
	bool b_loadedSprite = false;
	bool b_createdCollider = false;
};