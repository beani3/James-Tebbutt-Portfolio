/** @file collideComponent.hpp */
#pragma once
#include <glm/glm.hpp>
#include <entt/entt.hpp>
#include <components/transformComponent.hpp>

struct AABB
{
	AABB() : min(glm::vec3(0.f)), max(glm::vec3(0.f)) {}; //!< Default ctor
	/** Constructor which takes in a min and max value */
	AABB(glm::vec3 min, glm::vec3 max) : min(min), max(max) {}
	glm::vec3 min{glm::vec3(0.f)}; //!< Min point of the bounding box 
	glm::vec3 max{ glm::vec3(0.f)}; //!< Max point of bounding box
};

struct OBBCollider
{
	/** Constructor which takes half extents and entity */
	OBBCollider(const glm::vec3& he, entt::entity entt, TransformComponent transComp) : halfExtents(he), entity(entt), transformComp(transComp) {
		aabb = AABB(transComp.translation - he, transComp.translation + he);
		// float length = glm::length(he);
		// aabb = AABB(glm::vec3(-length), glm::vec3(length));
	}
	glm::vec3 halfExtents{ glm::vec3(0.f) }; //!< Half extents of the collider i.e. half height, width and depth
	entt::entity entity; //!< Entity which collider is attached to
	TransformComponent transformComp; //!< Transform component of entity with collider
	AABB aabb; //!< Axis aligned bounding box for OBB
};

struct SphereCollider
{
	/** Constructor which takes radii and entity */
	SphereCollider(float rad, entt::entity entt, TransformComponent transComp) : radius(rad), entity(entt), transformComp(transComp) {
		aabb = AABB(transComp.translation - rad, transComp.translation + rad);
		// aabb = AABB(glm::vec3(-rad), glm::vec3(rad));
	}
	float radius{ 0.f }; //!< Radius of sphere
	entt::entity entity; //!< Entity which collider is attached to
	TransformComponent transformComp; //!< Transform component of entity with collider
	AABB aabb; //!< Axis aligned bounding box for sphere
};