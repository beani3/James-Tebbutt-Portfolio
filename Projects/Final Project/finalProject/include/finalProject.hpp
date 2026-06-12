#pragma once
#include <glm/glm.hpp>
#include <rendering/Renderer.hpp>
#include <rendering/camera.hpp>
#include "rendering/sceneLoader.hpp"
#include <box2d/box2d.h>

enum GameStates { begin, inGame };

/**
 * @file finalProject.hpp
 * @brief Top-level application class for the final project demo.
 *
 * FinalProject owns the Renderer, the entt::registry scene and Box2D world.
 * It encapsulates level creation, input handling, animation, camera control
 * and the per-frame update/render lifecycle.
 *
 * Important notes:
 * - The renderer is used for loading textures and rendering; texture GLuints
 *   are obtained and bindless handles are used on RenderComponents.
 * - Physics uses the Box2D C API; world and bodies are referenced by id handles.
 */

/**
 * @brief Simple POD representing a platform used by the level generator.
 */
struct Platform {
	glm::vec4 colour{ glm::vec4(0.f) }; //!< Colour applied to the platform when rendered.
	glm::vec2 pos{ glm::vec4(0.f) }; //!< Position of the platform centre.
	glm::vec2 halfExtent{ glm::vec4(0.f) }; //!< Half extents (half width, half height) of the platform.
	glm::vec2 u{ glm::vec2(0.f, 1.f) }; //!< Default UV u-range (min,max) used for sprite selection.
	glm::vec2 v{ glm::vec2(0.f, 1.f) }; //!< Default UV v-range (min,max) used for sprite selection.
	uint64_t texHandle{ 0 }; //!< Bindless texture handle for the platform.
};

/**
 * @brief Application class encapsulating game systems and level state.
 *
 * Public API:
 * - FinalProject() - initializes world, loads textures and constructs the level.
 * - onRender(), onUpdate() - per-frame hooks invoked by the Application wrapper.
 *
 * Internally manages entity creation and component wiring (Render/Transform/Physics/Animation).
 */
class FinalProject {
public:
	/**
	 * @brief Construct the FinalProject instance.
	 *
	 * Initializes physics world, loads default textures and creates level.
	 */
	FinalProject();

	/** @brief Default destructor. */
	~FinalProject() = default;

	/** @brief Called to render the scene. */
	void onRender();

	/** @brief Called each frame to update physics, animation and input. */
	void onUpdate();

private:
	/** @brief Build initial level: player, platforms and tilemaps. */
	void generateLevel();

	/** @brief Poll input and apply physics impulses/forces. */
	void handleInput();

	/** @brief Select texture handle and frame counts based on animation state. */
	void handleAnimationState();

	/** @brief Advance animation timers and update UVs accordingly. */
	void updateAnimation();

	/** @brief Move camera in response to player movement. */
	void moveCamera();

	/** @brief Handle fail/reset conditions (e.g. falling out of world). */
	void handleFailStates();

	/** @brief Update internal timing (delta time). */
	void tick();

private:
	Renderer m_renderer; //!< The renderer responsible for drawing the scene
	Scene m_scene; //!< entt registry containing entities and components
	Camera m_camera; //!< Camera instance providing view and projection matrices for rendering
	SceneLoader m_sceneLoader; //!< Helper to load Tiled maps into the scene

	b2WorldDef m_worldDef; //!< Box2D world configuration

	std::vector<Platform> m_platforms; //!< Platforms constructed by generateLevel()

	/** Default values for colour and handle */
	glm::vec4 m_defaultColour{ glm::vec4(1.f) };
	glm::vec4 m_platformColour{ glm::vec4(1.0f, 0.41f, 0.71f, 1.0f) };
	uint64_t m_defaultTextureHandle{ 0 };
	uint64_t m_splashscreenTexHandle{ 0 };
	uint64_t m_runTexHandle{ 0 };
	uint64_t m_idleTexHandle{ 0 };

	glm::vec2 m_checkpoint{ glm::vec2(0.f) };

	/** Scene entities used in the prototype. */
	entt::entity m_splashscreen;
	entt::entity m_cameraEntt{ entt::null }; //!< Camera entity 
	entt::entity m_player{ entt::null }; //!< Player entity
	entt::entity m_platform{ entt::null }; //!< Temporary platform entity during setup

	b2WorldId m_worldID; //!< Handle to the created Box2D world

	GameStates m_gameStates{ GameStates::begin };
	/** Frame timing state. */
	float m_deltaTime{ 0.00001f };
	float m_currentFrame{ 0.f };
	float m_lastFrame{ 0.f };

	/** Physics simulation variables */
	float m_bounds{ 5.f }; //!< Horizontal bounds used by camera stepping
	float m_timer{ 0.f };
	int m_jumpCount{ 0 };
	int m_subStep{ 0 }; //!< Number of physics sub-steps per frame

	GLuint m_defaultTexture{ 0 }; //!< Default texture id loaded at startup

	bool m_canJump{ true }; //!< Double-jump gating flag
	bool m_canMove{ true };
};