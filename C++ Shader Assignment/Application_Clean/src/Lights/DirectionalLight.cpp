#pragma once
#include "Lights/DirectionalLight.h"

DirectionalLight::DirectionalLight(glm::vec3 colour, glm::vec3 direction, float ambient) :
	Light(colour),
	m_direction(direction),
	m_ambientFactor(ambient)
{}

void DirectionalLight::setLightUniforms(Shader* shader) {
	shader->setVec3("lightDirection", m_direction);
	shader->setVec3("lightColour", m_colour);
	shader->setFloat("ambientFactor", m_ambientFactor);
}