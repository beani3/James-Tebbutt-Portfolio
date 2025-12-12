#pragma once
#include <glad\glad.h>
#include <GLFW/glfw3.h>
#include "glm/common.hpp"
#include <glm/gtc/matrix_transform.hpp>
#include <vector>
#include <memory>
#include "Shader.h"

class Plane {
public:
	Plane(unsigned int diffuseTexture, unsigned int specularTexture, unsigned int normalTexture, float shine);
	~Plane();
	void setPlaneMatVals(Shader* shader);

	void setTransform(Shader* shader) { shader->setMat4("Model", m_transform); }
	void resetTransform() { m_transform = glm::mat4(1.0); }

	unsigned int& getVAO() { return m_VAO; }
	glm::mat4& getModelMatrix() { return m_transform; }
	unsigned int getIndiciesCount() { return planeIndicies.size(); }

	void scale(float scaleFactor);
	void rotate(float angle, glm::vec3 axis);
	void translate(glm::vec3 translation);
private:
	const float floorLevel = -2.0f;
	const float floorSize = 7.0f;
	void makeVAO();

	unsigned int m_VAO;
	glm::mat4 m_transform;
	unsigned int m_diffuseTexture;
	unsigned int m_specularTexture;
	unsigned int m_normalTexture;
	float m_shine;
	//float m_specularStrength;
	//glm::vec3 m_colour;

	std::vector<float> vertexData = {
		// xyz, normal, uv, tan
		-floorSize, floorLevel, -floorSize,  0.0, 1.0, 0.0,  0.0, 0.0,  1.0f, 0.0f, 0.0f,
		 floorSize, floorLevel, -floorSize,  0.0, 1.0, 0.0,  1.0, 0.0,  1.0f, 0.0f, 0.0f,
		 floorSize, floorLevel,  floorSize,  0.0, 1.0, 0.0,  1.0, 1.0,  1.0f, 0.0f, 0.0f,
		-floorSize, floorLevel,  floorSize,  0.0, 1.0, 0.0,  0.0, 1.0,  1.0f, 0.0f, 0.0f
	};

	std::vector<unsigned int> planeIndicies = {
		3, 2, 1,
		3, 1, 0
	};
};