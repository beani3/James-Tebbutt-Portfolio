#pragma once
#include "Lights/PointLight.h"#
#include <string>	

PointLight::PointLight(glm::vec3 col, glm::vec3 pos, glm::vec3 attn) :
	Light(col),
	m_position(pos),
	m_attentuation(attn)
{}

PointLight::~PointLight() {

}

void PointLight::setLightUniforms(Shader* shader, int i) {
	std::string sI = std::to_string(i);
	std::string baseName = "pointLight[" + sI + "]";

	shader->setVec3(baseName + ".col", m_colour);
	shader->setVec3(baseName + ".pos", m_position);
	shader->setVec3(baseName + ".attn", m_attentuation);
}