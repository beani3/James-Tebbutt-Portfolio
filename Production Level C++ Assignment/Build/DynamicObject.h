#include "GameObject.h"
#include <SFML/Graphics.hpp>
#include <string>
#pragma once

class DynamicObject : public GameObject {
public:
	DynamicObject();
	~DynamicObject();
	sf::Sprite& createObject(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size) override;
	void placeObject(sf::Sprite s_sprite, sf::Vector2f v2_pos, sf::RenderWindow& window) override;
	// Collider Creation and Logic
	sf::FloatRect& makeCollider(sf::Vector2f position, float f_width, float f_height);

	sf::Sprite& getSprite() { return s_sprite; }
private:
	sf::Sprite s_sprite;
	sf::Texture t_texture;
	sf::IntRect ir_sourceSprite;
protected:
	sf::FloatRect fr_collider;
};