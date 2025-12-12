#include "StaticObject.h"
#include <string>
#include <iostream>

StaticObject::StaticObject() {

}

StaticObject::~StaticObject() {}

/* Creates the static object from its
 * file location containing the texture and sprite
 */
sf::Sprite& StaticObject::createObject(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size) {
	if (!t_texture.loadFromFile(s_dir)) {
		std::cout << "Could not load texture" << std::endl;
		b_loaded = false;
	}

	ir_sourceSprite = sf::IntRect(i_left, i_top, v2_size.x, v2_size.y);
	b_loaded = true;
	return sf::Sprite(t_texture, ir_sourceSprite);
}

/* Places object at specified location
 * on the scene.
 */
void StaticObject::placeObject(sf::Sprite s_sprite, sf::Vector2f v2_pos, sf::RenderWindow& window) {
	s_sprite.setPosition(v2_pos);
	window.draw(s_sprite);
}

/************************
 * Collider Logic
 ************************/
sf::FloatRect& StaticObject::makeCollider(sf::Vector2f v2_position, float f_width, float f_height) {
	fr_collider = sf::FloatRect(v2_position.x - (f_width / 2), v2_position.y - (f_height / 2), f_width, f_height);
	return fr_collider;
}

void StaticObject::setColliderPos(float x, float y) {
	fr_collider.left = x - fr_collider.width / 2.f;
	fr_collider.top = y - fr_collider.height / 2.f;
}