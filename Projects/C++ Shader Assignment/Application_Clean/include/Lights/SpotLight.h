#pragma once
#include "PointLight.h"
#include "Shader.h"

class SpotLight : public PointLight {
public:
	SpotLight(glm::vec3 col, glm::vec3 pos, glm::vec3 attn, glm::vec3 dir, glm::vec2 radii);
	~SpotLight();

	void setLightUniforms(Shader* shader);
	
	void setColour(glm::vec3& col) { m_colour = col; }
	glm::vec3& getColour() { return m_colour; }

	void setPosition(glm::vec3& pos) { m_position = pos; }
	glm::vec3& getPosition() { return m_position; }

	void setAttentuation(glm::vec3& attn) { m_attentuation = attn; }
	glm::vec3& getAttentuation() { return m_attentuation; }

	void setDirection(glm::vec3& dir) { m_direction = dir; }
	glm::vec3& getDirection() { return m_direction; }

	void setRadii(glm::vec2& radii) { m_radii = radii; }
	glm::vec2& getRadii() { return m_radii; }
private:
	glm::vec3 m_colour;
	glm::vec3 m_position;
	glm::vec3 m_attentuation;
	glm::vec3 m_direction;
	glm::vec2 m_radii;
};