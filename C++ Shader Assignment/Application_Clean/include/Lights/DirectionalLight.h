#pragma once
#pragma once
#include "Light.h"
#include "Shader.h"

class DirectionalLight : public Light {
public:
	DirectionalLight(glm::vec3 colour, glm::vec3 direction, float ambient = 0.5);
	~DirectionalLight() {};
	// sets uniforms for lights in the shader
	void setLightUniforms(Shader* shader);

	// getters and setters
	void setDirection(glm::vec3& dir) { m_direction = dir; }
	void setAmbient(float& a) { m_ambientFactor = a; }
	glm::vec3& getDirection() { return m_direction; }
	float& getAmbientFactor() { return m_ambientFactor; }

private:
	glm::vec3 m_direction;
	float m_ambientFactor;
};