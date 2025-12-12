#include <SFML/Graphics.hpp>
#include "EnemyNPC.h"
#pragma once
class NPCBuilder {
public:
	virtual ~NPCBuilder() = default;
	/*void createNewNPC() {
		m_enemyNPC = std::make_unique<EnemyNPC>();
	}
	EnemyNPC* getNPC() {
		return m_enemyNPC.release();
	}*/
	virtual NPCBuilder& buildSpeed(float speed) = 0;
	virtual NPCBuilder& buildHealth(float health) = 0;
	virtual NPCBuilder& buildPos(sf::Vector2f pos) = 0;
	virtual NPCBuilder& buildScale(sf::Vector2f sf) = 0;
	virtual NPCBuilder& buildSprite(std::string s_dir, int i_left, int i_top, sf::Vector2f v2_size) = 0;
	virtual NPCBuilder& buildCollider(sf::Vector2f v2_position, float f_width, float f_height) = 0;
	virtual std::shared_ptr<EnemyNPC> build() = 0;
protected:
	std::shared_ptr<EnemyNPC> m_enemyNPC;
};	