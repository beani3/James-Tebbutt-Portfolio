#pragma once

#include <filesystem>
#include "rendering/renderer.hpp"
#include <box2d/box2d.h>

/**
 * @file sceneLoader.hpp
 * @brief Utility to load Tiled JSON maps into an entt::registry and Box2D world.
 *
 * SceneLoader uses the Renderer to load tile atlas textures and assigns bindless
 * texture handles to RenderComponents created for tiles. Object layers with
 * collider objects are converted into Box2D fixtures.
 */
class SceneLoader
{
public:
	/** @brief Default constructor. */
	SceneLoader() = default;

	/**
	 * @brief Load a Tiled JSON map into the provided scene and physics world.
	 * @param scene Registry to populate with entities/components.
	 * @param filepath Path to the Tiled JSON file describing the map.
	 * @param worldID Box2D world id to create colliders in.
	 */
	void LoadScene(Scene& scene, std::filesystem::path filepath, b2WorldId worldID);

private:
	Renderer m_renderer; //!< Local renderer instance used to load textures.
	
	/**
	 * @brief Resolve the image file referenced by a tileset JSON file.
	 * @param tilesetJsonPath Path to the tileset JSON file.
	 * @return filesystem::path Resolved image path (parent directory + image filename).
	 */
	std::filesystem::path GetTilesetPath(std::filesystem::path tilesetJsonPath);
};