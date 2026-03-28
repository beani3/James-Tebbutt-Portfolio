/** @file gameLayer.hpp */
#pragma once
#include <DemonRenderer.hpp>
#include "ImGui/bloomPanel.hpp"
#include "ImGui/textureSlotPanel.hpp"
#include "sharedData.hpp"
#include "rendering/cameraFrustum.hpp"

/**
 * @brief Main application layer for the Asteroid Belt demo.
 *
 * @details This Layer owns scenes, renderer and game-specific logic (level generation,
 * collision checks, input handling and ImGui panels). It is intended to be
 * registered with the application layer stack and receives per-frame update
 * and render callbacks via the Layer interface.
 */
class AsteriodBelt : public Layer
{
public:
	/**
	 * @brief Construct the game layer.
	 * @param win Reference to the GLFW window implementation for input/window ops.
	 * @param data Shared game data that is accessed/modified by this layer.
	 *
	 * @details The constructor should perform any lightweight initialization required to
	 * prepare the layer. Heavy resource creation should be done in onUpdate or
	 * deferred to avoid blocking the main thread during construction.
	 */
	AsteriodBelt(GLFWWindowImpl& win, SharedData& data);

private:
	/**
	 * @brief Render callback invoked by the engine.
	 *
	 * @details This function is const because rendering should not modify layer-owned
	 * state. All rendering is expected to be deterministic with respect to the
	 * current layer state.
	 */
	void onRender() const override;

	/**
	 * @brief Per-frame update callback.
	 * @param timestep Delta time in seconds since the last update.
	 *
	 * Update game logic, animations, physics and other time-dependent systems.
	 */
	void onUpdate(float timestep) override;

	/**
	 * @brief ImGui rendering callback.
	 *
	 * @details Called each frame to emit ImGui widgets associated with this layer (e.g.
	 * debug panels, options and runtime controls).
	 */
	void onImGUIRender() override;

	/**
	 * @brief Key press event handler.
	 * @param e Event payload containing key code and repeat count.
	 *
	 * Handle keyboard input relevant to the game (e.g. toggles, shortcuts).
	 */
	void onKeyPressed(KeyPressedEvent& e) override;

	/**
	 * @brief Procedurally generate / reset the level contents.
	 *
	 * Responsible for creating asteroids, waypoints, lights and populating the
	 * scene graph used by the renderer.
	 */
	void generateLevel();

	/**
	 * @brief Check for and resolve collisions with waypoints.
	 *
	 * Used to detect when the player reaches a waypoint and advance objectives.
	 */
	void checkWaypointCollisions();

	/**
	 * @brief Check for and resolve collisions between the ship and asteroids.
	 *
	 * Performs simple collision detection and applies game rules (damage,
	 * bounce, scoring, etc.) as appropriate.
	 */
	void checkAsteroidCollisions();

private:
	SharedData& m_gameData; //!< Reference to shared game data across layers.
	std::shared_ptr<Scene> m_mainScene; //!< Main 3D scene for gameplay rendering.
	std::shared_ptr<Scene> m_screenScene; //!< Screen-space / UI scene.
	std::vector<std::shared_ptr<Scene>> m_bloomScenes; //!< Scenes used for bloom extraction/passes.

	Renderer m_mainRenderer; //!< Renderer instance used for the main scene.
	CameraFrustum m_cameraFrustum; //!< Frustum used for culling objects against the camera.

	/** ImGui panels */
	BloomPanel m_bloomPanel; //!< Controls bloom parameters and extraction settings.
	TextureSlotPanel m_textureSlotPanel; //!< UI for managing texture slots.
	size_t m_wireframe{ false }; //!< Wireframe toggle (stored as size_t to match UI control).

	/** Entities stored in the scene */
	entt::entity camera{ entt::null }; //!< Camera entity
	entt::entity ship{ entt::null }; //!< Player ship entity
	entt::entity cube{ entt::null }; //!< Generic cube entity (debug/test)
	entt::entity asteroid{ entt::null }; //!< Representative asteroid entity
	entt::entity nextTarget{ entt::null }; //!< Next waypoint/target entity
	entt::entity skybox{ entt::null }; //!< Skybox entity

	const int m_numPointLights = 50; //!< Number of dynamic point lights used in the scene.
	glm::vec3 m_camToAstDist; //!< Cached vector from camera to nearest asteroid (for effects/culling).
	int m_cullCount{ 0 }; //!< Number of objects culled this frame (for stats/debug).
};