#include "DynamicObject.h"
#include <string>
#include <iostream>

DynamicObject::DynamicObject() {

}

DynamicObject::~DynamicObject() {}

/*
 * Creates the static object from its
 * file location containing the texture and sprite
 */
sf::Sprite& DynamicObject::createObject(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size) {
	if (!t_texture.loadFromFile(s_dir)) {
		std::cout << "Could not load texture" << std::endl;
	}

	ir_sourceSprite = sf::IntRect(i_left, i_top, v2_size.x, v2_size.y);
	s_sprite = sf::Sprite(t_texture, ir_sourceSprite);
	return s_sprite;
}

/* Not in use at the moment as all dynamic objects currently
 * set their own positions as well as rotations and scales in their
 * constructor. Causes "Object of abstract type class not allowed" error if 
 * this isn't here so here it is. Need to figure out the why behind that.
 */
void DynamicObject::placeObject(sf::Sprite s_sprite, sf::Vector2f v2_pos, sf::RenderWindow& window) {

}

/************************
 * Collider Logic
 ************************/
sf::FloatRect& DynamicObject::makeCollider(sf::Vector2f v2_position, float f_width, float f_height) {
	fr_collider = sf::FloatRect(v2_position.x - (f_width / 2), v2_position.y - (f_height / 2), f_width, f_height);
	return fr_collider;
}