#pragma once
#include <glm/glm.hpp>
#include <glm/ext/matrix_clip_space.hpp>

struct Camera {
	glm::mat4 view{ glm::mat4(1.f) }; //!< 4x4 view matrix for 2D transformations (translation, rotation, scale)
	glm::mat4 projection{ glm::mat4(1.f) }; //!< 4x4 projection matrix for 2D orthographic projection
	void updateView(const glm::mat4& cameraTransform) {
		view = glm::inverse(cameraTransform); //!< Invert the camera transform to get the view matrix
	}
};