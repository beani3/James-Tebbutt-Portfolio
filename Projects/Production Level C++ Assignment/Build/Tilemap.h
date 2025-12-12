#pragma once
#include "StaticObject.h"

class Tilemap : public StaticObject {
public:
	Tilemap();

	/* Getter for the global bounds of the 
	 * tilemap for use of keeping player on it.
	 */
	sf::FloatRect getCollider() { return fr_collider; }
	void setCollider(sf::FloatRect collider) { fr_collider = collider; }

	/* Declaring Function(s) in Tile.cpp
	 */
	void drawTilemap(sf::RenderWindow& window);
private:
	/* Declaring variables for tilemap and defining some.
	 */
	int i_tileWidth = 64;
	int i_tileHeight = 64;

	// Temporary storage for IntRect and Sprite
	sf::IntRect ir_current;
	sf::Sprite s_sprite;
};