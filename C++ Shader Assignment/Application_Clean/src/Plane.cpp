#pragma once
#include "Plane.h"
#include "myScene.h"

Plane::Plane(unsigned int diffuseTexture, unsigned int specularTexture, unsigned int normalTexture, float shine) :
	m_diffuseTexture(diffuseTexture),
	m_specularTexture(specularTexture),
	m_normalTexture(normalTexture),
	m_shine(shine)
{
	makeVAO();
	resetTransform();
}

Plane::~Plane() {

}

void Plane::scale(float scaleFactor) {
	m_transform = glm::scale(m_transform, glm::vec3(scaleFactor));
}

void Plane::rotate(float angle, glm::vec3 axis) {
	m_transform = glm::rotate(m_transform, angle, axis);
}

void Plane::translate(glm::vec3 translation) {
	m_transform = glm::translate(m_transform, translation);
}

void Plane::makeVAO() {
	unsigned int VBO;
	unsigned int EBO;

	glCreateBuffers(1, &VBO);
	glNamedBufferStorage(VBO, sizeof(float) * vertexData.size(), vertexData.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateBuffers(1, &EBO); // create element buffer
	glNamedBufferStorage(EBO, sizeof(unsigned int) * planeIndicies.size(), planeIndicies.data(), GL_DYNAMIC_STORAGE_BIT);

	glCreateVertexArrays(1, &m_VAO);
	glVertexArrayVertexBuffer(m_VAO, 0, VBO, 0, sizeof(float) * 11);
	glVertexArrayElementBuffer(m_VAO, EBO);

	glEnableVertexArrayAttrib(m_VAO, 0);
	glEnableVertexArrayAttrib(m_VAO, 1);
	glEnableVertexArrayAttrib(m_VAO, 2);
	glEnableVertexArrayAttrib(m_VAO, 3);

	glVertexArrayAttribFormat(m_VAO, 0, 3, GL_FLOAT, GL_FALSE, 0); // start reading with 0 at stride 

	glVertexArrayAttribFormat(m_VAO, 1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3);

	glVertexArrayAttribFormat(m_VAO, 2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 6);

	glVertexArrayAttribFormat(m_VAO, 3, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8);

	glVertexArrayAttribBinding(m_VAO, 0, 0);
	glVertexArrayAttribBinding(m_VAO, 1, 0);
	glVertexArrayAttribBinding(m_VAO, 2, 0);
	glVertexArrayAttribBinding(m_VAO, 3, 0);
}

void Plane::setPlaneMatVals(Shader* shader) {
	shader->use();
	shader->setFloat("shine", m_shine);
	shader->setInt("diffuseMap", 0);
	shader->setInt("specularMap", 1);
	shader->setInt("normalMap", 2);

	glBindTextureUnit(0, m_diffuseTexture);
	glBindTextureUnit(1, m_specularTexture);
	glBindTextureUnit(2, m_normalTexture);
}