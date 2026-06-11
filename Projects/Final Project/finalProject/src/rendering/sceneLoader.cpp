/**
 * @file sceneLoader.cpp
 * @brief Load Tiled JSON maps into an entt::registry and Box2D world.
 *
 * SceneLoader reads a Tiled-exported JSON file, creates RenderComponent and
 * TransformComponent entities for tile layers and creates Box2D fixtures for
 * object layers that represent colliders.
 *
 * The loader uses Renderer to load tile atlas textures and obtain bindless
 * handles to assign to RenderComponents.
 */

#pragma once
#include <sstream>
#include <fstream>
#include "rendering/sceneLoader.hpp"
#include "rendering/renderer.hpp"
#include "finalProject.hpp"
#include <nlohmann/json.hpp>
#include <components/renderComponent.hpp>
#include <components/transformComponent.hpp>
#include <components/physicsComponent.hpp>
#include <components/tilemapComponent.hpp>
#include <entt/entt.hpp>
#include <iostream>

/**
 * @brief Load a Tiled JSON file and populate the provided scene and Box2D world.
 *
 * @param scene entt::registry to populate with entities/components.
 * @param filepath Path to the Tiled JSON map file.
 * @param worldID Box2D world id in which colliders should be created.
 *
 * Notes:
 * - Tile textures are loaded via Renderer::loadTexture; the renderer returns a
 *   GLuint which the loader converts to a bindless handle via Renderer::getHandle.
 * - Coordinates are scaled by a hard-coded scale factor (1/100) to convert
 *   tile pixels to world units.
 * - Logic is implemented but not functional.
 * - Inspired by Tiled parser written by fellow student, Jamie Schooling.
 */
void SceneLoader::LoadScene(Scene& scene, std::filesystem::path filepath, b2WorldId worldID) {

	std::ifstream stream(filepath);
	std::stringstream sstream;
	sstream << stream.rdbuf();

	nlohmann::json data = nlohmann::json::parse(sstream.str());

	for (auto& layer : data["layers"])
	{
		if (layer["type"] == "tilelayer")
		{
			std::filesystem::path tilesetPath = GetTilesetPath("./assets/textures/kennyAsset/renderer-platformer-tilemap.json");
			std::string tempStr = tilesetPath.string();
			TextureDescription tilemapTexDesc = {
				.src = tempStr.c_str(),
			};
			
			GLuint tilemapTexture = m_renderer.loadTexture(tilemapTexDesc);
			int tilesetWidth = (tilemapTexDesc.getWidth() / data["tilewidth"].get<int>());
			uint64_t tilemapHandle = m_renderer.getHandle(tilemapTexture);

			int id = 0;
			int sortingIndex = 0;
			for (auto& property : layer["properties"])
			{
				if (property["name"] == "id")
				{
					id = property["value"];
				}
				else if (property["name"] == "sortingIndex")
				{
					sortingIndex = property["value"];
				}
			}

			std::vector<int> tiles = layer["data"];	

			float sf = 1.f / 100.f;
			glm::vec2 tileSize = { data["tilewidth"].get<int>() * sf, data["tileheight"].get<int>() * sf };

			//for (unsigned int i = 6; i < 7; i++)
			for (unsigned int i = 0; i < layer["width"]; ++i)
			{
				//for (unsigned int j = 0; j < 1; ++j)
				for (unsigned int j = 0; j < layer["height"]; ++j)
				{
					int tileNumber = (tiles[i + j * layer["width"]]);
					if (tileNumber < 0) continue;
					tileNumber -= 1;

					float texWidth = (float)tilemapTexDesc.getWidth();
					float texHeight = (float)tilemapTexDesc.getHeight();

					int tileWidth = data["tilewidth"];
					int tileHeight = data["tileheight"];

					int tileX = tileNumber % tilesetWidth;
					int tileY = tileNumber / tilesetWidth;

					float u0 = (tileX * tileWidth) / texWidth;
					float u1 = ((tileX + 1) * tileWidth) / texWidth;

					float v0 = 1.0f - ((tileY + 1) * tileHeight) / texHeight;
					float v1 = 1.0f - (tileY * tileHeight) / texHeight;

					entt::entity tile = scene.create();
					auto& renderComp = scene.emplace<RenderComponent>(tile);

					/*renderComp.halfExtent = tileSize * 0.5f;
					renderComp.pos = { i * tileSize.x, ((layer["height"].get<int>() - 1 - j) * tileSize.y) };
					
					renderComp.u = { 0, 1 };
					renderComp.v = { 0, 1 };

					renderComp.colour = glm::vec4(1.f);
					renderComp.textureHandle = tilemapHandle;

					auto& transformComp = scene.emplace<TransformComponent>(tile);*/
				}
			}
		}
		else if (layer["type"] == "objectgroup")
		{
			TextureDescription platformTexDesc = {
				.src = "./assets/textures/missing_texture.png",
				.width = 512,
				.height = 512,
				.channels = 4
			};

			GLuint platformTexHandle = m_renderer.getHandle(m_renderer.loadTexture(platformTexDesc));
			float sf = 1.f / 100.f;

			for (auto& object : layer["objects"])
			{
				if (object["type"] == "Collider")
				{
					int id = 0;
					for (auto& property : layer["properties"])
					{
						if (property["name"] == "id")
						{
							id = property["value"];
						}
					}

					/*b2BodyDef groundDef = b2DefaultBodyDef();
					groundDef.position = { object["x"].get<float>() * sf, object["y"].get<float>() * sf };
					b2BodyId groundID = b2CreateBody(worldID, &groundDef);
					b2Polygon groundBox = b2MakeBox(object["width"].get<int>() * sf, object["height"].get<int>() * sf);
					b2ShapeDef groundShapeDef = b2DefaultShapeDef();
					b2CreatePolygonShape(groundID, &groundShapeDef, &groundBox);

					entt::entity platform = scene.create();

					auto& renderComp = scene.emplace<RenderComponent>(platform);

					renderComp.colour = glm::vec4(1.f);
					renderComp.pos = { groundDef.position.x, groundDef.position.y };
					renderComp.halfExtent = glm::vec2(object["width"].get<float>() * sf, object["height"].get<float>() * sf);
					renderComp.textureHandle = platformTexHandle;

					auto& transformComp = scene.emplace<TransformComponent>(platform);*/
				}
			}
		}
	}
}

/**
 * @brief Resolve and return the image path declared inside a tileset JSON.
 *
 * @param tilesetJsonPath Path to a tileset JSON file (Tiled format).
 * @return filesystem::path The resolved image path (tileset JSON parent directory + image filename).
 */
std::filesystem::path SceneLoader::GetTilesetPath(std::filesystem::path tilesetJsonPath)
{
	std::ifstream stream(tilesetJsonPath);
	std::stringstream sstream;
	sstream << stream.rdbuf();

	nlohmann::json data = nlohmann::json::parse(sstream.str());
	return tilesetJsonPath.parent_path() / data["image"];
}
