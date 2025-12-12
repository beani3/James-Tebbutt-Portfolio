#pragma once
#pragma once
#include <glm/glm.hpp>

class Light {
public:
	Light(glm::vec3 colour) : m_colour(colour) {}
	~Light() {};
protected:
	glm::vec3& getColour() { return m_colour; }
	void setColour(glm::vec3 col) { m_colour = col; }

	glm::vec3 m_colour;
};
