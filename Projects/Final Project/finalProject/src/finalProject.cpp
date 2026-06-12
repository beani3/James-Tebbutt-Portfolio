/**
 * @file finalProject.cpp
 * @brief FinalProject implementation: game setup, update and rendering.
 *
 * This translation unit implements the FinalProject class which:
 * - Owns the entt::registry scene, Renderer and Box2D world.
 * - Creates player, platforms and optional tile maps.
 * - Steps physics, synchronizes transforms, handles input, animation and camera movement.
 *
 * Notes:
 * - All OpenGL calls assume a valid GL context is current on the calling thread.
 * - Box2D usage here uses the C API (b2WorldId / b2BodyId).
 * - The renderer provides texture-loading and bindless handle helpers; texture
 *   object lifetime is managed by the caller/creator in this module (not the renderer).
 */

#include "finalProject.hpp"
#include <components/renderComponent.hpp>
#include <components/transformComponent.hpp>
#include <components/physicsComponent.hpp>
#include <components/animationComponent.hpp>
#include <windows/window.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <core/input.hpp>
#include <iostream>

/**
 * @brief Construct FinalProject, create physics world and initial scene.
 *
 * Responsibilities:
 * - Initialize Box2D world using b2DefaultWorldDef().
 * - Load default textures and acquire bindless handles.
 * - Create optional splash screen and call generateLevel() to populate entities.
 * - Configure the renderer's view and projection uniforms.
 *
 * Throws: None directly, but underlying resource loaders may throw std::runtime_error.
 */
FinalProject::FinalProject() {
	
	m_worldDef = b2DefaultWorldDef();
	m_worldDef.gravity = {0.0f, -9.81f};
	m_worldID = b2CreateWorld(&m_worldDef);
	
	TextureDescription defaultTexDesc = {
		.src = "./assets/textures/default_texture.png",
		.width = 512,
		.height = 512,
		.channels = 4
	};
	m_defaultTextureHandle = m_renderer.getHandle(m_renderer.loadTexture(defaultTexDesc)); //!< Get texture handle for bindless textures (not used in shader yet)

	TextureDescription splashscreenTexDesc = {
		.src = "./assets/textures/splashscreen.png",
		.width = 512,
		.height = 512,
		.channels = 4
	};
	m_splashscreenTexHandle = m_renderer.getHandle(m_renderer.loadTexture(splashscreenTexDesc));

	m_splashscreen = m_scene.create();

	if (m_gameStates == GameStates::begin) {
		auto& renderComp = m_scene.emplace<RenderComponent>(m_splashscreen);
		auto& transformComp = m_scene.emplace<TransformComponent>(m_splashscreen);

		glm::vec2 pos = { 0.f, 0.f };
		glm::vec2 size = { 10.5f, 8.f };
		renderComp.pos = { pos.x, pos.y };
		renderComp.halfExtent = size * 0.5f;

		renderComp.textureHandle = m_splashscreenTexHandle;
		renderComp.colour = m_defaultColour;
	}

	generateLevel();

	m_camera.view = glm::mat4(1.f); //!< Identity view matrix (no transformation)
	m_camera.projection = glm::ortho(-5.12f, 5.12f, -4.f, 4.f);

	GLuint viewLoc = glGetUniformLocation(m_renderer.getShaderID(), "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_camera.view)); //!< Set the view matrix uniform in the shader

	GLuint projLoc = glGetUniformLocation(m_renderer.getShaderID(), "projection");
	glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(m_camera.projection)); //!< Set the projection matrix uniform in the shader
}

/**
 * @brief Render callback forwarded to Renderer.
 *
 * Calls Renderer::render with the internal scene registry.
 */
void FinalProject::onRender() {
	m_renderer.render(m_scene);
}

/**
 * @brief Per-frame update: physics, transforms, input, animation and camera.
 *
 * Steps the Box2D world, synchronizes the player TransformComponent with the
 * physics body, then runs game logic only when the game state is `inGame`.
 *
 * Sequence:
 *  - Step physics (with m_subStep subdivisions).
 *  - If in splash state and SPACE pressed, remove splash and enter inGame.
 *  - If inGame: update transforms, camera, input, failure handling, animation.
 *  - Finally update timing with tick().
 */
void FinalProject::onUpdate() {
	m_subStep = 4;
	b2World_Step(m_worldID, m_deltaTime, m_subStep);

	auto& transformComp = m_scene.get<TransformComponent>(m_player);
	auto& physicsComp = m_scene.get<PhysicsComponent>(m_player);

	if (InputSystem::get().isKeyPressed(GLFW_KEY_ENTER) && m_gameStates == GameStates::begin) {
		m_scene.destroy(m_splashscreen);
		m_gameStates = GameStates::inGame;
	}

	if (m_gameStates == GameStates::inGame) {

		b2Vec2 bodyPos = b2Body_GetPosition(physicsComp.id);
		transformComp.translation = { bodyPos.x, bodyPos.y };
		transformComp.recalc();

		moveCamera();
		handleInput();
		handleFailStates();
		handleAnimationState();
		updateAnimation();
	}

	tick(); //!< Ensure this is at the bottom of the update loop to calculate delta time for the next frame
}

/**
 * @brief Populate scene with player, platforms and sprite resources.
 *
 * Loads sprite-sheets and registers their bindless handles, creates the player
 * entity with physics and animation components, and builds a set of platform
 * entities used in the level.
 *
 * Exceptions: texture loading may throw; callers should ensure appropriate error handling.
 */
void FinalProject::generateLevel() {

	TextureDescription idleTexDesc = {
			.src = "./assets/textures/player-spriteSheets/Tuxedo-Idle.png",
			.width = 192,
			.height = 16,
			.channels = 4
	};

	m_idleTexHandle = m_renderer.getHandle(m_renderer.loadTexture(idleTexDesc));

	TextureDescription runTexDesc = {
		.src = "./assets/textures/player-spriteSheets/Tuxedo-Run.png",
		.width = 96,
		.height = 16,
		.channels = 4
	};

	m_runTexHandle = m_renderer.getHandle(m_renderer.loadTexture(runTexDesc));

	// m_sceneLoader.LoadScene(m_scene, "./assets/textures/kennyAsset/renderer-demo-map.json", m_worldID);

	m_player = m_scene.create();
	{
		glm::vec2 pos = { 0.f, 0.f };
		glm::vec2 size = { 0.5f, 0.5f };

		auto& animationComp = m_scene.emplace<AnimationComponent>(m_player);
		animationComp.animState = animStates::Idle;
		animationComp.frames = 12.f;

		auto& renderComp = m_scene.emplace<RenderComponent>(m_player);

		renderComp.pos = { pos.x, pos.y };
		renderComp.halfExtent = size * 0.5f;

		renderComp.textureHandle = m_idleTexHandle;
		renderComp.colour = m_defaultColour;

		auto& physicsComp = m_scene.emplace<PhysicsComponent>(m_player);

		physicsComp.bodyDef = b2DefaultBodyDef();
		physicsComp.bodyDef.type = b2_dynamicBody;
		physicsComp.bodyDef.position = { -4.5f, 0.f };
		physicsComp.id = b2CreateBody(m_worldID, &physicsComp.bodyDef);

		b2Polygon rigidBody = b2MakeBox(renderComp.halfExtent.x, renderComp.halfExtent.y);

		physicsComp.shapeDef = b2DefaultShapeDef();
		physicsComp.shapeDef.density = 1.f;
		physicsComp.shapeDef.material.friction = 0.6f;

		b2CreatePolygonShape(physicsComp.id, &physicsComp.shapeDef, &rigidBody);

		auto& transformComp = m_scene.emplace<TransformComponent>(m_player);

		b2Vec2 bodyPos = b2Body_GetPosition(physicsComp.id);
		transformComp.translation = glm::vec2(bodyPos.x, bodyPos.y);
		transformComp.rotation = glm::radians(0.f);
		transformComp.scale = glm::vec2(1.f);
		transformComp.recalc(); //!< Recalculate transform matrix based on translation, rotation and scale

		m_checkpoint = { bodyPos.x, bodyPos.y };
	}

	TextureDescription platformTexDesc = {
		.src = "./assets/textures/platform.png",
		.width = 512,
		.height = 512,
		.channels = 4
	};

	GLuint platformTexHandle = m_renderer.getHandle(m_renderer.loadTexture(platformTexDesc));

	{
		glm::vec2 size{ 2.f, 5.f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { -4.5f, -3.f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.5f, 5.f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { -2.0f, -2.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 4.5f, 4.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { -0.75f, 3.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 2.f, 0.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 0.5f, -3.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.f, 0.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 3.f, -2.75f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 2.f, 3.f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 5.5f, -3.25f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.f, 0.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 7.5f, -1.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.f, 0.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 9.f, -0.55f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.f, 0.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 7.f, 0.25f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.5f, 0.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 9.5f, 1.f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.5f, 7.f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 12.f, -1.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.5f, 1.5f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 15.f, -3.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 1.5f, 5.f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 15.f, 1.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	{
		glm::vec2 size{ 9.f, 2.f };
		Platform platform = {
			.colour = m_platformColour,
			.pos = { 20.5f, -3.5f },
			.halfExtent = size * 0.5f,
			.texHandle = platformTexHandle
			//!< Using default UVs so not include in platformer construction
		};

		m_platforms.push_back(platform);
	}

	for (int i = 0; i < m_platforms.size(); i++) {
		m_platform = m_scene.create();

		auto& renderComp = m_scene.emplace<RenderComponent>(m_platform);

		renderComp.colour = m_platforms[i].colour;
		renderComp.pos = { 0.f, 0.f };
		renderComp.halfExtent = m_platforms[i].halfExtent;
		renderComp.textureHandle = m_platforms[i].texHandle;

		auto& physicsComp = m_scene.emplace<PhysicsComponent>(m_platform);

		physicsComp.bodyDef = b2DefaultBodyDef();
		physicsComp.bodyDef.type = b2_staticBody;
		physicsComp.bodyDef.position = { m_platforms[i].pos.x, m_platforms[i].pos.y };
		physicsComp.id = b2CreateBody(m_worldID, &physicsComp.bodyDef);
		b2Polygon groundBox = b2MakeBox(renderComp.halfExtent.x, renderComp.halfExtent.y);
		b2ShapeDef groundShapeDef = b2DefaultShapeDef();
		b2CreatePolygonShape(physicsComp.id, &groundShapeDef, &groundBox);

		auto& transformComp = m_scene.emplace<TransformComponent>(m_platform);

		b2Vec2 bodyPos = b2Body_GetPosition(physicsComp.id);
		transformComp.translation = glm::vec2(bodyPos.x, bodyPos.y);
		transformComp.recalc();

		auto& animationComp = m_scene.emplace<AnimationComponent>(m_platform);
	}

	m_cameraEntt = m_scene.create();
	auto& transformComp = m_scene.emplace<TransformComponent>(m_cameraEntt);
}

/**
 * @brief Poll input and apply forces/impulses to the player body.
 *
 * - Reads InputSystem for movement and jump keys.
 * - Applies linear impulses for jumping and forces for horizontal movement.
 * - Manages a double-jump gate with m_canJump and m_jumpCount timers.
 * - Flips player scale when changing facing direction (applies immediately).
 *
 * All input polling clears InputSystem::m_keyStates to avoid stuck keys.
 */
void FinalProject::handleInput() {

	auto& physicsComp = m_scene.get<PhysicsComponent>(m_player);
	auto& animationComp = m_scene.get<AnimationComponent>(m_player);
	auto& transformComp = m_scene.get<TransformComponent>(m_player);
	auto& renderComp = m_scene.get<RenderComponent>(m_player);

	glm::vec2 targetVel = { 0.f, 0.f };
	float speed = 2.f;

	bool keyPressed{ false };
	if (InputSystem::get().isKeyPressedPoll(GLFW_KEY_A) && m_canMove || InputSystem::get().isKeyPressedPoll(GLFW_KEY_LEFT) && m_canMove) {
		targetVel.x -= speed;
		keyPressed = true;
	}
	if (InputSystem::get().isKeyPressedPoll(GLFW_KEY_D) && m_canMove || InputSystem::get().isKeyPressedPoll(GLFW_KEY_RIGHT) && m_canMove) {
		targetVel.x += speed;
		keyPressed = true;
	}
	if (InputSystem::get().isKeyPressed(GLFW_KEY_SPACE) && m_canJump) {
		b2Body_ApplyLinearImpulseToCenter(physicsComp.id, b2Vec2(0.f, 2.05f), true);
		m_jumpCount++;
		
		if (m_jumpCount == 1) {
			m_canJump = false;
		}
	}
	if (!m_canJump) {
		m_timer += m_deltaTime;

		if (m_timer >= 0.5f) {
			m_canJump = true;
			m_jumpCount = 0;
			m_timer = 0.f;
		}
	}
	if (InputSystem::get().isKeyPressed(GLFW_KEY_R)) {
		b2Body_SetTransform(physicsComp.id, b2Vec2(-4.5f, 0.f), b2Body_GetRotation(physicsComp.id));
		b2Body_SetLinearVelocity(physicsComp.id, b2Vec2(0.f));
		m_canMove = false;

		if (!m_canMove) {
			m_timer += m_deltaTime;

			if (m_timer > 0.5f) {
				m_canMove = true;
				m_timer = 0.f;
			}
		}
	}
	InputSystem::get().m_keyStates.clear(); //!< Prevents stuck input register

	auto b2Vel = b2Body_GetLinearVelocity(physicsComp.id);
	targetVel.y = b2Vel.y;
	glm::vec2 dv = targetVel - glm::vec2(b2Vel.x, b2Vel.y);
	glm::vec2 a = dv / m_deltaTime;
	
	glm::vec2 force = a * b2Body_GetMass(physicsComp.id);
	b2Vec2 f(force.x, force.y);

	if(keyPressed) b2Body_ApplyForceToCenter(physicsComp.id, f, true);

	if (std::abs(b2Vel.x < -0.1f)) {
		transformComp.scale = glm::vec2(-1, 1);
		transformComp.recalc();
	}
	else {
		transformComp.scale = glm::vec2(1.f, 1.f);
		transformComp.recalc();
	}

	if (std::abs(b2Vel.x) > 0.1f) {
		animationComp.animState = animStates::Running;
	} else {
		animationComp.animState = animStates::Idle;
	}
}

/**
 * @brief Map animation state to sprite-sheet handles and frames.
 *
 * Updates RenderComponent.textureHandle and AnimationComponent.frames to
 * choose the appropriate sprite sheet and frame count for the current state.
 */
void FinalProject::handleAnimationState()
{
	auto& physicsComp = m_scene.get<PhysicsComponent>(m_player);
	auto& animationComp = m_scene.get<AnimationComponent>(m_player);
	auto& renderComp = m_scene.get<RenderComponent>(m_player);

	switch (animationComp.animState) {
	case animStates::Idle:

		renderComp.textureHandle = m_idleTexHandle;
		animationComp.frames = 12.f;
		renderComp.u = { 0.f, 1.f / animationComp.frames };
		break;

	case animStates::Running:
		
		renderComp.textureHandle = m_runTexHandle;
		animationComp.frames = 6.f;
		renderComp.u = { 0.f, 1.f / animationComp.frames };
		break;
	}
}

/**
 * @brief Advance sprite animation frames based on elapsed time.
 *
 * - Advances the animation timer.
 * - Wraps currentFrame when it reaches frames.
 * - Updates the RenderComponent's UV range (u/v) to select the correct frame.
 */
void FinalProject::updateAnimation() {

	auto& animationComp = m_scene.get<AnimationComponent>(m_player);
	auto& renderComp = m_scene.get<RenderComponent>(m_player);

	animationComp.timer += m_deltaTime;

	if (animationComp.timer >= animationComp.frameTime) {
		animationComp.timer -= animationComp.frameTime;
		animationComp.currentFrame++;

		if (animationComp.currentFrame >= animationComp.frames) {
			animationComp.currentFrame = 0;
		}
	}

	float frameWidth = 1.0f / static_cast<float>(animationComp.frames);

	float uMin = animationComp.currentFrame * frameWidth;
	float uMax = uMin + frameWidth;

	renderComp.u = { uMin, uMax };
	renderComp.v = { 0.0f, 1.0f };
}

/**
 * @brief Move camera based on player position and upload view uniform.
 *
 * The camera moves in discrete steps when the player crosses the bounds
 * threshold; the view matrix is then uploaded to the renderer shader.
 */
void FinalProject::moveCamera() {
	auto& physicsComp = m_scene.get<PhysicsComponent>(m_player);
	auto& renderComp = m_scene.get<RenderComponent>(m_player);
	auto& cameraTransform = m_scene.get<TransformComponent>(m_cameraEntt);
	b2Vec2 bodyPos = b2Body_GetPosition(physicsComp.id);

	if (b2Body_GetPosition(physicsComp.id).x - renderComp.halfExtent.x > m_bounds) {
		cameraTransform.translation += glm::vec2(10.f, 0.f);
		cameraTransform.recalc();
		m_bounds += 10.f;

		m_checkpoint = { bodyPos.x + 0.1f, bodyPos.y };
	}
	if (b2Body_GetPosition(physicsComp.id).x - renderComp.halfExtent.x < m_bounds - 10.f) {
		cameraTransform.translation -= glm::vec2(10.f, 0.f);
		cameraTransform.recalc();
		m_bounds -= 10.f;

		m_checkpoint = { bodyPos.x + 0.1f, bodyPos.y };
	}

	m_camera.updateView(cameraTransform.transform);
	GLuint viewLoc = glGetUniformLocation(m_renderer.getShaderID(), "view");
	glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(m_camera.view)); //!< Set the view matrix uniform in the shader
}

/**
 * @brief Reset player to checkpoint when falling out of bounds.
 *
 * If player's body Y position drops below a safety threshold, the body is
 * teleported back to the last recorded checkpoint and given a small impulse.
 */
void FinalProject::handleFailStates() {
	auto& physicsComp = m_scene.get<PhysicsComponent>(m_player);

	if (b2Body_GetPosition(physicsComp.id).y < -5.f) {
		b2Body_SetTransform(physicsComp.id, { m_checkpoint.x, m_checkpoint.y }, b2Body_GetRotation(physicsComp.id));
		b2Body_SetLinearVelocity(physicsComp.id, b2Vec2(0.f));
		m_canMove = false;
	}

	if (b2Body_GetPosition(physicsComp.id).y > 5.f) {
		b2Body_SetTransform(physicsComp.id, { m_checkpoint.x, m_checkpoint.y }, b2Body_GetRotation(physicsComp.id));
		b2Body_SetLinearVelocity(physicsComp.id, b2Vec2(0.f));
		m_canMove = false;
	}

	if (!m_canMove) {
		m_timer += m_deltaTime;

		if (m_timer > 0.5f) {
			m_canMove = true;
			m_timer = 0.f;
		}
	}
}

/**
 * @brief Update per-frame timing values.
 *
 * Updates m_currentFrame, computes m_deltaTime and stores the time for the next tick.
 * Uses GLFW's glfwGetTime() to retrieve a high-resolution time value in seconds.
 */
void FinalProject::tick() {
	m_currentFrame = glfwGetTime();
	m_deltaTime = m_currentFrame - m_lastFrame;
	m_lastFrame = m_currentFrame;
}