#include "AnimationManager.h"
#include <string>
#include <iostream>

AnimationManager::AnimationManager() {

}

AnimationManager::~AnimationManager() {

}

void AnimationManager::playAnimation(std::string s_dir, int i_frames, int i_left, int i_top, sf::Vector2f v2_size)
{
	t_texture.loadFromFile(s_dir);
	ir_sourceSprite = sf::IntRect(i_left, i_top, v2_size.x, v2_size.y);
	// s_target = sf::Sprite(t_texture, ir_sourceSprite);

	for (int i = 0; i < i_frames; i++) {
		ir_sourceSprite = sf::IntRect(v2_size.x * i, 0, v2_size.x, v2_size.y);
		v_sourceSpritePos.push_back(ir_sourceSprite);
	
		if (c_clock.getElapsedTime().asSeconds() > 1.0f) {
			if (v_sourceSpritePos[i].left == v2_size.x * i_frames) {
				ir_sourceSprite = v_sourceSpritePos[0];
			}
			else {
				ir_sourceSprite = v_sourceSpritePos[i];
			}

			s_target.setTextureRect(v_sourceSpritePos[i]);
		}
	}	
}