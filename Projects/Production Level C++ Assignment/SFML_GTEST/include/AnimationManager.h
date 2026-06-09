#include <SFML/Graphics.hpp>
#pragma once

class AnimationManager {
public:
	AnimationManager();
	~AnimationManager();

	void playAnimation(std::string s_dir, int i_frames,int i_left, int i_top, sf::Vector2f v2_size);

	/* Getters and Setters for Animation Testing
	 */
	sf::IntRect& getIntRect() { return ir_sourceSprite; }
	std::vector<sf::IntRect>& getSpriteVec() { return v_sourceSpritePos; }
	sf::Sprite& getTarget() { return s_target; }
private:
	sf::Texture t_texture;
	sf::Sprite s_sprite;
	sf::IntRect ir_sourceSprite;
	std::vector<sf::IntRect> v_sourceSpritePos;
	sf::Clock c_clock;
	sf::Sprite s_target;
	int i_vecSize = 0;
};