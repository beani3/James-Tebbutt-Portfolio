/** @file cameraFrustum.hpp*/
#pragma once
#include "camera.hpp"
#include <components/collideComponent.hpp>
#include <array>

/** @class CameraFrustum
 *  @brief Represents the six planes and eight corner points of a camera frustum.
 *
 * @detail The frustum is computed from a Camera's view and projection matrices and can
 * be used to perform conservative visibility tests (e.g. AABB-frustum intersection).
 */
class CameraFrustum
{
public:
	CameraFrustum() = default; //!< Default CTor

	/**
	 * @brief Construct frustum planes/corners from a Camera.
	 * @param cam Camera used to build the frustum (uses cam.projection * cam.view).
	 */
	CameraFrustum(const Camera& cam);

	/**
	 * @brief Test whether an axis-aligned bounding box intersects the frustum.
	 * @param aabb Axis-aligned bounding box to test.
	 * @return true if the box intersects (or is inside) the frustum, false if the box is fully outside.
	 */
	bool intersects(const AABB& aabb);
private:
	std::array<glm::vec4, 6> m_planes;  //!< Frustum planes in view-space (plane equation coefficients)
	std::array<glm::vec4, 8> m_corners; //!< Frustum corner positions in world space
};