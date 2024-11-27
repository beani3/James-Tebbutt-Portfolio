#pragma once
#include "Light.h"
#include "Shader.h"

class PointLight : public Light {
public:
	PointLight(glm::vec3 col, glm::vec3 pos, glm::vec3 attn);
	~PointLight();

	void setLightUniforms(Shader* shader, int i);

	void setPosition(glm::vec3& pos) { m_position = pos; }
	glm::vec3& getPosition() { return m_position; }

	void setAttentuation(glm::vec3& attn) { m_attentuation = attn; }
	glm::vec3& getAttentuation() { return m_attentuation; }
private:
	glm::vec3 m_position;
	glm::vec3 m_attentuation;
};