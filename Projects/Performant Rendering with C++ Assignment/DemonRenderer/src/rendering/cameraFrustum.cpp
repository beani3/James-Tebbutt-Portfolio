/** @file cameraFrustum.cpp */
#include "rendering/cameraFrustum.hpp"
#include <entt/entt.hpp>
#include <components/transformComponent.hpp>
#include <iostream>

/**
 * @brief Build frustum planes and corners from a camera's projection*view matrix.
 * @param cam Camera with projection and view matrices.
 *
 * @details The plane equations are extracted by using the transpose of the MVP matrix,
 * following the common extraction technique. Corners are reconstructed by
 * transforming normalized device coordinates back into world space.
 */
CameraFrustum::CameraFrustum(const Camera& cam) {
	glm::mat4 mvp = cam.projection * cam.view;

	glm::mat4 mvpt = glm::transpose(mvp);
	m_planes[0] = glm::vec4(mvpt[3] + mvpt[0]);
	m_planes[1]	= glm::vec4(mvpt[3] - mvpt[0]);
	m_planes[2]	= glm::vec4(mvpt[3] + mvpt[1]);
	m_planes[3]	= glm::vec4(mvpt[3] - mvpt[1]);
	m_planes[4]	= glm::vec4(mvpt[3] + mvpt[2]);
	m_planes[5]	= glm::vec4(mvpt[3] - mvpt[2]);

	std::array<glm::vec4, 8> ndc;
	ndc[0] = glm::vec4(-1, -1, -1, 1);
	ndc[1] = glm::vec4(1, -1, -1, 1);
	ndc[2] = glm::vec4(1, 1, -1, 1);
	ndc[3] = glm::vec4(-1, 1, -1, 1);
	ndc[4] = glm::vec4(-1, -1, 1, 1);
	ndc[5] = glm::vec4(1, -1, 1, 1);
	ndc[6] = glm::vec4(1, 1, 1, 1);
	ndc[7] = glm::vec4(-1, 1, 1, 1);
	
	const glm::mat4 invVp = glm::inverse(mvp);

	for (int i = 0; i != 8; i++) {
		const glm::vec4 c = invVp * ndc[i];
		m_corners[i] = c / c.w;
	}
}

/**
 * @brief Test whether an axis-aligned bounding box intersects the frustum.
 * @param aabb Axis-aligned bounding box to test.
 * @return true if the box intersects (or is inside) the frustum, false if the box is fully outside.
 *
 * Uses a conservative test:
 *  - First, check each frustum plane against the 8 box corners; if all corners are outside any plane => no intersection.
 *  - Then check if all frustum corners lie on one side of the AABB along any axis => no intersection.
 */
bool CameraFrustum::intersects(const AABB& aabb) {
	for (int i = 0; i < 6; i++) {
			int r = 0;
		r += (dot(m_planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.min.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.min.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.max.x, aabb.min.y, aabb.max.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.min.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0) ? 1 : 0;
		r += (dot(m_planes[i], glm::vec4(aabb.max.x, aabb.max.y, aabb.max.z, 1.0f)) < 0.0) ? 1 : 0;
		// std::cout << r << " : Planes" << std::endl;
		if (r == 8) return false;
	}

	/** Check frustum outside/inside box */
	int r = 0;
	// std::cout << r << " : Corners" << std::endl;
	r = 0; for (int i = 0; i < 8; i++) r += ((m_corners[i].x > aabb.max.x) ? 1 : 0); if (r == 8) return false;
	// std::cout << r << " : Corners" << std::endl;
	r = 0; for (int i = 0; i < 8; i++) r += ((m_corners[i].x < aabb.min.x) ? 1 : 0); if (r == 8) return false;
	// std::cout << r << " : Corners" << std::endl;
	r = 0; for (int i = 0; i < 8; i++) r += ((m_corners[i].y > aabb.max.y) ? 1 : 0); if (r == 8) return false;
	// std::cout << r << " : Corners" << std::endl;
	r = 0; for (int i = 0; i < 8; i++) r += ((m_corners[i].y < aabb.min.y) ? 1 : 0); if (r == 8) return false;
	// std::cout << r << " : Corners" << std::endl;
	r = 0; for (int i = 0; i < 8; i++) r += ((m_corners[i].z > aabb.max.z) ? 1 : 0); if (r == 8) return false;
	// std::cout << r << " : Corners" << std::endl;
	r = 0; for (int i = 0; i < 8; i++) r += ((m_corners[i].z < aabb.min.z) ? 1 : 0); if (r == 8) return false;
	// std::cout << r << " : Corners" << std::endl;

	return true;
}