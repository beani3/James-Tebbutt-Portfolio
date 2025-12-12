#pragma once
#include "Lights/SpotLight.h"

SpotLight::SpotLight(glm::vec3 col, glm::vec3 pos, glm::vec3 attn, glm::vec3 dir, glm::vec2 radii) :
	PointLight(col, pos, attn),
	m_direction(dir),
	m_radii(radii)
{}

SpotLight::~SpotLight() {

}

void SpotLight::setLightUniforms(Shader* shader) {
	shader->setVec3("slightColour", m_colour);
	shader->setVec3("slightPosition", m_position);
	shader->setVec3("sAttentuation", m_attentuation);
	shader->setVec3("sDirection", m_direction);
	shader->setVec2("sRadii", m_radii);
}