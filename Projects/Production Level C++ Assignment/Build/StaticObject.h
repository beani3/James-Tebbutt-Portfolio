#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <string>
#pragma once

class StaticObject : public GameObject {
public:
	StaticObject();
	~StaticObject();
	sf::Sprite& createObject(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size) override;
	void placeObject(sf::Sprite s_sprite, sf::Vector2f v2_pos, sf::RenderWindow& window) override;
	// Testing Function(s)
	bool objectTest(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size);
	// Collider Creation and Logic
	sf::FloatRect& makeCollider(sf::Vector2f position, float f_width, float f_height);
	void setColliderPos(float x, float y);

	sf::Sprite& getSprite() { return s_sprite; }
	// Getter(s) for Testing 
	bool getLoaded() { return b_loaded; }
private:
	sf::Sprite s_sprite;
	sf::Texture t_texture;
	sf::IntRect ir_sourceSprite;
	bool b_loaded = false;
protected:
	sf::FloatRect fr_collider;
};