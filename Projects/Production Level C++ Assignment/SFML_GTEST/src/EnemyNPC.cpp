#include "EnemyNPC.h"
#include <SFML/Graphics.hpp>
#include <SFML/System.hpp>
#include <SFML/Window.hpp>
#include <iostream>

void EnemyNPC::setColliderPos(float x, float y) {
	fr_collider.left = x - fr_collider.width / 2.f;
	fr_collider.top = y - fr_collider.height / 2.f;

	// std::cout << fr_collider.left << std::endl;
	// std::cout << fr_collider.top << std::endl;
}

void EnemyNPC::draw(sf::RenderWindow& window) {
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