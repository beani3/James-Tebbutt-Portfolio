#pragma once
#include <glm/glm.hpp>

/**
 * @file renderComponent.hpp
 * @brief Component describing renderable quad data for an entity.
 *
 * This component stores compact per-entity data that the renderer will read
 * when building draw batches. It is intended for simple axis-aligned quads
 * described by a center position and half extents. Texture usage is via a
 * bindless texture handle.
 */
struct RenderComponent {
	/**
	 * @brief Vertex colour used when rendering.
	 *
	 * RGBA colour applied to the quad. Default is white (1.0).
	 */
	glm::vec4 colour{ glm::vec4(1.f) };

	/**
	 * @brief Center position of the quad in world (or local) 2D space.
	 *
	 * Interpreted as the quad's centre point. Coordinate space (world or local)
	 * depends on how the application places entities.
	 */
	glm::vec2 pos;

	glm::vec2 u{ 0.f, 1.f };
	glm::vec2 v{ 0.f, 1.f };
	/**
	 * @brief Half the width and height of the quad.
	 *
	 * The full quad size is (halfExtent * 2). Using half extents simplifies
	 * collision/placement math and keeps the vertex generation symmetric
	 * around the `pos` centre.
	 *
	 * Example:
	 * - halfExtent = {0.5f, 0.25f} produces a quad 1.0 units wide and 0.5 units tall.
	 */
	glm::vec2 halfExtent;

	/**
	 * @brief Bindless texture handle for the texture used by this quad.
	 *
	 * A value of 0 indicates "no texture". The handle is produced by the
	 * renderer via `Renderer::getHandle`. Handles marked resident must be
	 * made non-resident before shutdown (the renderer currently tracks this).
	 */
	uint64_t textureHandle{ 0 };
};