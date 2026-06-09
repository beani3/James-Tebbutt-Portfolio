#include "Tilemap.h"
#include <iostream>

Tilemap::Tilemap() {
	s_sprite = createObject("../Assets/TinySwords/Terrain/Ground/Tilemap_Flat.png", 0, 0, sf::Vector2f(0, 0)); 
}

void Tilemap::drawTilemap(sf::RenderWindow& window) {
	const int i_rowAmount = 10;
	const int i_columnAmount = 10;

	int i_tilemap[i_rowAmount][i_columnAmount] = {
		{0, 1, 1, 1, 1, 1, 1, 1, 1, 2},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{3, 4, 4, 4, 4, 4, 4, 4, 4, 5},
		{6, 7, 7, 7, 7, 7, 7, 7, 7, 8},
	};

	fr_collider = makeCollider(sf::Vector2f(s_sprite.getPosition().x - 287, s_sprite.getPosition().y - 287), 64 * i_rowAmount, 64 * i_columnAmount);
	setCollider(fr_collider);

	/* Sets the IntRect to the correct 
	 * target tile.
	 */
	for (int i = 0; i < i_rowAmount; i++) {
		for (int j = 0; j < i_columnAmount; j++) {
			if (i_tilemap[i][j] == 0) {
				// Top Left
				ir_current = sf::IntRect({ 0, 0 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 1) {
				// Top Middle
				ir_current = sf::IntRect({ 64, 0 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 2) {
				// Top Right
				ir_current = sf::IntRect({ 128, 0 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 3) {
				// Middle Left
				ir_current = sf::IntRect({ 0, 64 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 4) {
				// Middle
				ir_current = sf::IntRect({ 64, 64 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 5) {
				// Middle Right
				ir_current = sf::IntRect({ 128, 64 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 6) {
				// Bottom Left
				ir_current = sf::IntRect({ 0, 128 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 7) {
				// Bottom Middle
				ir_current = sf::IntRect({ 64, 128 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 8) {
				// Bottom Right
				ir_current = sf::IntRect({ 128, 128 }, { 64, 64 });
			}
			else if (i_tilemap[i][j] == 9) {
				// Blank
				ir_current = sf::IntRect({ 256, 256 }, { 64, 64 });
			}

			/* Apply the texture to the target sprite 
			 * @ set its correct position relative to
			 * the spritesheet.
			 */
			s_sprite.setTextureRect(ir_current);
			s_sprite.setOrigin(s_sprite.getTextureRect().width / 2, s_sprite.getTextureRect().height / 2);
			s_sprite.setPosition(sf::Vector2f((j * i_tileWidth) + 350, (i * i_tileHeight) + 75));
			placeObject(s_sprite, s_sprite.getPosition(), window);
		

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
	}
}