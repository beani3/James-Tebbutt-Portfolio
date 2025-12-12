#include "PlayerCharacter.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>

PlayerCharacter::PlayerCharacter(float f_speed, float f_health, sf::Vector2f v2_pos, float f_angle, sf::Vector2f v2_sf) {
	this->f_speed = f_speed;
	this->f_health = f_health;
	
	/* Creates the object as a dynamic object 
	 * @ sets the sprite, position (transform), rotation, and scale.
	 * Order is important. Remember TRanSform. Thanks Connor :)
	 */
	s_sprite = createObject("../Assets/TinySwords/Factions/Knights/Troops/Warrior/Purple/Warrior_Purple-Idle.png", 0, 0, sf::Vector2f(192, 192));
	setSprite(s_sprite);
	s_sprite.setOrigin(s_sprite.getTextureRect().width / 2, s_sprite.getTextureRect().height / 2);
	s_sprite.setPosition(v2_pos);
	s_sprite.setRotation(f_angle);
	s_sprite.setScale(v2_sf.x, v2_sf.y);
	fr_collider = makeCollider(s_sprite.getPosition(), 64, 64);
	setCollider(fr_collider);
}

void PlayerCharacter::takeDamage(float f_amount) {
	setHealth(getHealth() - f_amount);
}

void PlayerCharacter::move(float dt) {
	sf::Vector2f v2_movement(0.0f, 0.0f);

	if (sf::Keyboard::isKeyPressed(sf::Keyboard::W) || sf::Keyboard::isKeyPressed(sf::Keyboard::Up)) {
		v2_movement.y -= (f_speed * dt);
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::S) || sf::Keyboard::isKeyPressed(sf::Keyboard::Down)) {
		v2_movement.y += f_speed * dt;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::A) || sf::Keyboard::isKeyPressed(sf::Keyboard::Left)) {
		v2_movement.x -= f_speed * dt;
	}
	if (sf::Keyboard::isKeyPressed(sf::Keyboard::D) || sf::Keyboard::isKeyPressed(sf::Keyboard::Right)) {
		v2_movement.x += f_speed * dt;
	}

	s_sprite.move(v2_movement);
}

void PlayerCharacter::setColliderPos(float x, float y) {
	fr_collider.left = x - fr_collider.width / 2.f;
	fr_collider.top = y - fr_collider.height / 2.f;

	// std::cout << fr_collider.left << std::endl;
	// std::cout << fr_collider.top << std::endl;
}

void PlayerCharacter::draw(sf::RenderWindow& window) {
	window.draw(s_sprite);
	/************************
	 * Debug Collider
	 ************************/
	/*sf::RectangleShape rs_debugSquare;
	rs_debugSquare.setPosition(getCollider().getPosition());
	rs_debugSquare.setSize(getCollider().getSize());
	rs_debugSquare.setFillColor(sf::Color::Transparent);
	rs_debugSquare.setOutlineThickness(2);
	rs_debugSquare.setOutlineColor(sf::Color::Red);
	window.draw(rs_debugSquare);*/
}