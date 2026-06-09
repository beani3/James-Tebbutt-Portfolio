#include <string>
#include <SFML/Graphics.hpp>
#pragma once

class GameObject {
public:
	GameObject() = default;
	~GameObject() = default;
	virtual sf::Sprite& createObject(std::string dir, int i_left, int i_top, sf::Vector2f v2_size) = 0; // Position of the object in Sprite Sheet
	virtual void placeObject(sf::Sprite s_sprite, sf::Vector2f v2_pos, sf::RenderWindow& window) = 0; // Position of the object on the scene 
};