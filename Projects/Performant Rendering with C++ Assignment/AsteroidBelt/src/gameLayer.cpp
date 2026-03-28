/** @file gameLayer.cpp */
#include "gameLayer.hpp"
#include "controller.hpp"
#include "rotation.hpp"
#include <numeric> //!< For std::iota
#include <glm/gtc/matrix_transform.hpp>
#include "core/randomiser.hpp" //!< For random number generation
#include "tracy/Tracy.hpp"
#include <entt/entt.hpp>
#include <components/renderComponent.hpp>
#include <components/transformComponent.hpp>
#include <components/collideComponent.hpp>
#include <components/order.hpp>
#include <iostream>

AsteriodBelt::AsteriodBelt(GLFWWindowImpl& win, SharedData& data) : Layer(win), m_gameData(data)
{
	m_gameData.closeAsteroids.reserve(20);
	m_gameData.closeTargets.reserve(10);

	LOD_data screenLODs;
	screenLODs[0] = { 0, 6 };
	screenLODs[1] = { 0, 6 };
	screenLODs[2] = { 0, 6 };

	generateLevel();

	/*************************
	*  Main Render Pass
	**************************/

	RenderPass mainPass;
	FBOLayout typicalLayout = {
		{nullptr, AttachmentType::ColourHDR, true, true},
		{nullptr , AttachmentType::Depth, true, false}
	};

	mainPass.scene = m_mainScene;
	mainPass.parseScene();
	mainPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout);
	//mainPass.target = std::make_shared<FBO>();
	mainPass.camera.projection = glm::perspective(45.f, m_winRef.getWidthf() / m_winRef.getHeightf(), 0.1f, 2000.f);
	mainPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	// mainPass.camera.updateView(m_mainScene->m_actors.at(cameraIdx).transform);
	mainPass.camera.updateView(m_mainScene->m_entities.get<TransformComponent>(camera).transform);

	mainPass.setCachedValue("b_camera", "u_view", mainPass.camera.view);
	mainPass.setCachedValue("b_camera", "u_projection", mainPass.camera.projection);
	//// mainPass.setCachedValue("b_camera", "u_viewPos", m_mainScene->m_actors.at(cameraIdx).translation);
	mainPass.setCachedValue("b_camera", "u_viewPos", m_mainScene->m_entities.get<TransformComponent>(camera).translation);

	mainPass.setCachedValue("b_lights", "dLight.colour", m_mainScene->m_directionalLights.at(0).colour);
	mainPass.setCachedValue("b_lights", "dLight.direction", m_mainScene->m_directionalLights.at(0).direction);

	for (size_t i = 0; i < 50; i++) {
		mainPass.setCachedValue("b_lights", "pLights[" + std::to_string(i) + "].colour", m_mainScene->m_pointLights.at(i).colour);
		mainPass.setCachedValue("b_lights", "pLights[" + std::to_string(i) + "].position", m_mainScene->m_pointLights.at(i).position);
		mainPass.setCachedValue("b_lights", "pLights[" + std::to_string(i) + "].constants", m_mainScene->m_pointLights.at(i).constants);
	}

	m_mainRenderer.addRenderPass(mainPass);

	/*************************
	*  Bloom
	**************************/

	/** General setup */

	/** Values common across all passes */
	const std::vector<uint32_t> screenIndices = { 0,1,2,2,3,0 };

	VBOLayout screenQuadLayout = {
		{GL_FLOAT, 3},
		{GL_FLOAT, 2}
	};

	FBOLayout screenLayout = {
		{nullptr, AttachmentType::ColourHDR, true, false}
	};

	std::vector<float> screenVertices = {
		/** Position										UV */
		0.f, 0.f, 0.f,										0.f, 1.f,
		m_winRef.getWidthf(),	0.f, 0.f,					1.f, 1.f,
		m_winRef.getWidthf(), m_winRef.getHeightf(), 0.f,	1.f, 0.f,
		0.f, m_winRef.getHeightf(), 0.f,					0.f, 0.f,
	};

	std::shared_ptr<VAO> screenVAO = std::make_shared<VAO>(screenIndices);
	screenVAO->addVertexBuffer(screenVertices, screenQuadLayout);

	glm::mat4 screenProj = glm::ortho(0.f, m_winRef.getWidthf(), m_winRef.getHeightf(), 0.f);
	ViewPort screenViewport = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	entt::entity quad;

	std::shared_ptr<Texture> downsampleTexture;
	TextureDescription downsampleTD;
	downsampleTD.type = TextureDataType::HDR;
	downsampleTD.width = m_winRef.getWidth();
	downsampleTD.height = m_winRef.getHeight();
	downsampleTD.channels = 4;
	downsampleTD.wrapMethod = GL_CLAMP_TO_EDGE;
	downsampleTD.minFilterMethod = GL_LINEAR_MIPMAP_LINEAR;
	downsampleTD.isMipMapped = true;

	downsampleTexture = std::make_shared<Texture>(downsampleTD);

	std::shared_ptr<Texture> upsampleTexture;
	TextureDescription upsampleTD;
	upsampleTD.type = TextureDataType::HDR;
	upsampleTD.width = m_winRef.getWidth();
	upsampleTD.height = m_winRef.getHeight();
	upsampleTD.channels = 4;
	upsampleTD.wrapMethod = GL_CLAMP_TO_EDGE;
	upsampleTD.minFilterMethod = GL_LINEAR_MIPMAP_LINEAR;
	upsampleTD.isMipMapped = true;

	upsampleTexture = std::make_shared<Texture>(upsampleTD);

	ShaderDescription thresholdShaderDesc;
	thresholdShaderDesc.type = ShaderType::rasterization;
	thresholdShaderDesc.vertexSrcPath = "./assets/shaders/Bloom/Vert.glsl";
	thresholdShaderDesc.fragmentSrcPath = "./assets/shaders/Bloom/ThresholdFrag.glsl";

	std::shared_ptr<Shader> thresholdShader;
	thresholdShader = std::make_shared<Shader>(thresholdShaderDesc);

	ShaderDescription downBlurShaderDesc;
	downBlurShaderDesc.type = ShaderType::rasterization;
	downBlurShaderDesc.vertexSrcPath = "./assets/shaders/Bloom/Vert.glsl";
	downBlurShaderDesc.fragmentSrcPath = "./assets/shaders/Bloom/DownBlurFrag.glsl";

	std::shared_ptr<Shader> downBlurShader;
	downBlurShader = std::make_shared<Shader>(downBlurShaderDesc);

	ShaderDescription upFilterShaderDesc;
	upFilterShaderDesc.type = ShaderType::rasterization;
	upFilterShaderDesc.vertexSrcPath = "./assets/shaders/Bloom/Vert.glsl";
	upFilterShaderDesc.fragmentSrcPath = "./assets/shaders/Bloom/upFilterFrag.glsl";

	std::shared_ptr<Shader> upFilterShader;
	upFilterShader = std::make_shared<Shader>(upFilterShaderDesc);


	std::array<RenderPass, 11> bloomPasses;
	size_t bloomPassIdx{ 0 };
	float divisor{ 1.f };

	/** Set up the Bloom Panel */
	m_bloomPanel = BloomPanel(&m_mainRenderer,downsampleTexture, upsampleTexture);

	/** Bloom Threshold pass */
	{
		auto& thresholdPass = bloomPasses[bloomPassIdx];

		std::shared_ptr<Material> thresholdMaterial;
		thresholdMaterial = std::make_shared<Material>(thresholdShader, "");

		thresholdPass.camera.projection = glm::ortho(0.f, m_winRef.getWidthf(), m_winRef.getHeightf(), 0.f);
		thresholdMaterial->setValue("u_projection", thresholdPass.camera.projection);
		thresholdMaterial->setValue("u_albedoMap", mainPass.target->getTarget(0));
		thresholdMaterial->setValue("u_threshold", m_bloomPanel.getThreshold());

		std::shared_ptr<Scene> thresholdScene = std::make_shared<Scene>();

		/** Scope so render component has limited scope */
		{
			quad = thresholdScene->m_entities.create(); //!< Create an entity in the scene

			/** Add render component and reference to it in the scope */
			auto& renderComp = thresholdScene->m_entities.emplace<RenderComponent>(quad);
			/** Set geometry and material */
			renderComp.geometry = screenVAO;
			renderComp.material = thresholdMaterial;
			renderComp.LOD_data = screenLODs;

			/** Add transform component */
			thresholdScene->m_entities.emplace<TransformComponent>(quad);
		}

		FBOLayout thresholdLayout = {
			{downsampleTexture, AttachmentType::ColourHDR, true, true, 0}
		};
		std::shared_ptr<FBO>thresholdFBO = std::make_shared<FBO>(m_winRef.getSize(), thresholdLayout);

		thresholdPass.scene = thresholdScene;
		thresholdPass.parseScene();
		thresholdPass.target = thresholdFBO;
		thresholdPass.viewPort = screenViewport;

		m_mainRenderer.addRenderPass(thresholdPass);
	}

	/** Downscale and blur passes(5 of them) */
	for (size_t i = 1; i < 6; i++)
	{
		divisor += divisor;
		bloomPassIdx++;

		float w = ceil(m_winRef.getWidthf() / divisor);
		float h = ceil(m_winRef.getHeightf() / divisor);
		uint32_t w_i = static_cast<uint32_t>(std::roundf(w));
		uint32_t h_i = static_cast<uint32_t>(std::roundf(h));

		std::vector<float> mipVertices = {
			/** Position	UV */
			0.f, 0.f, 0.f,	0.f, 1.f,
			w,	0.f, 0.f,	1.f, 1.f,
			w, h, 0.f,		1.f, 0.f,
			0.f, h, 0.f,	0.f, 0.f,
		};

		glm::vec2 UVperPixels = { 1.f / w, 1.f / h };

		auto mipVAO = std::make_shared<VAO>(screenIndices);
		mipVAO->addVertexBuffer(mipVertices, screenQuadLayout);

		FBOLayout mipLayout = {
			{downsampleTexture, AttachmentType::ColourHDR, true, true, static_cast<uint8_t>(i)}
		};

		auto mipFBO = std::make_shared<FBO>(glm::ivec2(w_i, h_i), mipLayout);

		auto& downBlurPass = bloomPasses[bloomPassIdx];

		downBlurPass.camera.projection = glm::ortho(0.f, w, h, 0.f);
		auto downBlurMaterial = std::make_shared<Material>(downBlurShader, "");
		downBlurMaterial->setValue("u_projection", downBlurPass.camera.projection);
		downBlurMaterial->setValue("u_albedoMap", downsampleTexture);
		downBlurMaterial->setValue("u_UVperPixel", UVperPixels);
		downBlurMaterial->setValue("u_mip", static_cast<int32_t>(i - 1));

		auto mipScene = std::make_shared<Scene>();

		/** Scope so render component has limited scope */
		{
			quad = mipScene->m_entities.create(); //!< Create an entity in the scene

			/** Add render component and reference to it in the scope */
			auto& renderComp = mipScene->m_entities.emplace<RenderComponent>(quad);
			/** Set geometry and material */
			renderComp.geometry = mipVAO;
			renderComp.material = downBlurMaterial;
			renderComp.LOD_data = screenLODs;

			/** Add transform component */
			mipScene->m_entities.emplace<TransformComponent>(quad);
		}

		downBlurPass.scene = mipScene;
		downBlurPass.parseScene();
		downBlurPass.target = mipFBO;
		downBlurPass.viewPort = { 0, 0, w_i, h_i };

		m_mainRenderer.addRenderPass(downBlurPass);
	}

	/** Upfilter passes, first one is slightly different */
	{
		divisor /= 2;
		bloomPassIdx++;

		float w = ceil(m_winRef.getWidthf() / divisor);
		float h = ceil(m_winRef.getHeightf() / divisor);
		uint32_t w_i = static_cast<uint32_t>(std::roundf(w));
		uint32_t h_i = static_cast<uint32_t>(std::roundf(h));

		std::vector<float> mipVertices = {
			/** Position	UV */
			0.f, 0.f, 0.f,	0.f, 1.f,
			w,	0.f, 0.f,	1.f, 1.f,
			w, h, 0.f,		1.f, 0.f,
			0.f, h, 0.f,	0.f, 0.f,
		};

		glm::vec2 UVperPixels = { 1.f / w, 1.f / h };
		UVperPixels += UVperPixels;

		auto mipVAO = std::make_shared<VAO>(screenIndices);
		mipVAO->addVertexBuffer(mipVertices, screenQuadLayout);

		FBOLayout mipLayout = {
			{upsampleTexture, AttachmentType::ColourHDR, true, true, static_cast<uint8_t>(4)}
		};

		auto mipFBO = std::make_shared<FBO>(glm::ivec2(w_i, h_i), mipLayout);

		auto& upFilterPass = bloomPasses[bloomPassIdx];

		upFilterPass.camera.projection = glm::ortho(0.f, w, h, 0.f);
		auto upFilterMaterial = std::make_shared<Material>(upFilterShader, "");
		upFilterMaterial->setValue("u_projection", upFilterPass.camera.projection);
		upFilterMaterial->setValue("u_previousMipTex", downsampleTexture);
		upFilterMaterial->setValue("u_currentMipTex", downsampleTexture);
		upFilterMaterial->setValue("u_previousMip", 5);
		upFilterMaterial->setValue("u_currentMip", 4);
		upFilterMaterial->setValue("u_texelSize", UVperPixels);

		auto mipScene = std::make_shared<Scene>();

		/** Scope so render component has limited scope */
		{
			quad = mipScene->m_entities.create(); //!< Create an entity in the scene

			/** Add render component and reference to it in the scope */
			auto& renderComp = mipScene->m_entities.emplace<RenderComponent>(quad);
			/** Set geometry and material */
			renderComp.geometry = mipVAO;
			renderComp.material = upFilterMaterial;
			renderComp.LOD_data = screenLODs;

			/** Add transform component */
			mipScene->m_entities.emplace<TransformComponent>(quad);
		}

		upFilterPass.scene = mipScene;
		upFilterPass.parseScene();
		upFilterPass.target = mipFBO;
		upFilterPass.viewPort = { 0, 0, w_i, h_i };

		m_mainRenderer.addRenderPass(upFilterPass);
	}

	/** Upfilter passes 2..n */
	for(int32_t i = 3; i >= 0; i--)
	{
		divisor /= 2;
		bloomPassIdx++;

		float w = ceil(m_winRef.getWidthf() / divisor);
		float h = ceil(m_winRef.getHeightf() / divisor);
		uint32_t w_i = static_cast<uint32_t>(std::roundf(w));
		uint32_t h_i = static_cast<uint32_t>(std::roundf(h));

		std::vector<float> mipVertices = {
			/** Position	UV */
			0.f, 0.f, 0.f,	0.f, 1.f,
			w,	0.f, 0.f,	1.f, 1.f,
			w, h, 0.f,		1.f, 0.f,
			0.f, h, 0.f,	0.f, 0.f,
		};

		glm::vec2 UVperPixels = { 1.f / w, 1.f / h };
		UVperPixels += UVperPixels;

		auto mipVAO = std::make_shared<VAO>(screenIndices);
		mipVAO->addVertexBuffer(mipVertices, screenQuadLayout);

		FBOLayout mipLayout = {
			{upsampleTexture, AttachmentType::ColourHDR, true, true, static_cast<uint8_t>(i)}
		};

		auto mipFBO = std::make_shared<FBO>(glm::ivec2(w_i, h_i), mipLayout);

		auto& upFilterPass = bloomPasses[bloomPassIdx];

		upFilterPass.camera.projection = glm::ortho(0.f, w, h, 0.f);
		auto upFilterMaterial = std::make_shared<Material>(upFilterShader, "");
		upFilterMaterial->setValue("u_projection", upFilterPass.camera.projection);
		upFilterMaterial->setValue("u_previousMipTex", upsampleTexture);
		upFilterMaterial->setValue("u_currentMipTex", downsampleTexture);
		upFilterMaterial->setValue("u_previousMip", i+1);
		upFilterMaterial->setValue("u_currentMip", i);
		upFilterMaterial->setValue("u_texelSize", UVperPixels);

		auto mipScene = std::make_shared<Scene>();

		/** Scope so render component has limited scope */
		{
			quad = mipScene->m_entities.create(); //!< Create an entity in the scene

			/** Add render component and reference to it in the scope */
			auto& renderComp = mipScene->m_entities.emplace<RenderComponent>(quad);
			/** Set geometry and material */
			renderComp.geometry = mipVAO;
			renderComp.material = upFilterMaterial;
			renderComp.LOD_data = screenLODs;

			/** Add transform component */
			mipScene->m_entities.emplace<TransformComponent>(quad);
		}

		upFilterPass.scene = mipScene;
		upFilterPass.parseScene();
		upFilterPass.target = mipFBO;
		upFilterPass.viewPort = { 0, 0, w_i, h_i };

		m_mainRenderer.addRenderPass(upFilterPass);
	}

	/*************************
	*  Screen Pass
	**************************/

	ShaderDescription screenShaderDesc;
	screenShaderDesc.type = ShaderType::rasterization;
	screenShaderDesc.vertexSrcPath = "./assets/shaders/Composition/Vert.glsl";
	screenShaderDesc.fragmentSrcPath = "./assets/shaders/Composition/Frag.glsl";

	std::shared_ptr<Shader> screenShader;
	screenShader = std::make_shared<Shader>(screenShaderDesc);

	std::shared_ptr<Material> screenQuadMaterial;
	screenQuadMaterial = std::make_shared<Material>(screenShader);
	screenQuadMaterial->setValue("u_albedoMap", mainPass.target->getTarget(0));
	screenQuadMaterial->setValue("u_bloomMap", upsampleTexture);
	screenQuadMaterial->setValue("u_bloomStrength", m_bloomPanel.getStrength());

	m_screenScene.reset(new Scene);

	/** Scope so render component has limited scope */
	{
		quad = m_screenScene->m_entities.create(); //!< Create an entity in the scene

		/** Add render component and reference to it in the scope */
		auto& renderComp = m_screenScene->m_entities.emplace<RenderComponent>(quad);
		/** Set geometry and material */
		renderComp.geometry = screenVAO;
		renderComp.material = screenQuadMaterial;
		renderComp.LOD_data = screenLODs;

		/** Add transform component */
		m_screenScene->m_entities.emplace<TransformComponent>(quad);
	}

	RenderPass screenPass;
	screenPass.scene = m_screenScene;
	screenPass.parseScene();
	screenPass.target = std::make_shared<FBO>(); //!< Default FBO
	screenPass.viewPort = screenViewport;  //!< Same as Bloom threshold pass
	screenPass.camera.projection = screenProj; //!< Same as Bloom threshold pass
	screenPass.setCachedValue("b_camera2D", "u_view", screenPass.camera.view);
	screenPass.setCachedValue("b_camera2D", "u_projection", screenPass.camera.projection);

	m_mainRenderer.addRenderPass(screenPass);
}

void AsteriodBelt::onRender() const
{
	ZoneScopedN("OnRender");

	glEnable(GL_DEPTH_TEST); //!< For RenderDoc compatibility
	glDepthMask(GL_TRUE); //!< For RenderDoc compatibility
	m_mainRenderer.render();
}

void AsteriodBelt::onUpdate(float timestep)
{
	ZoneScopedN("OnUpdate");

	if (m_gameData.state == GameState::running) {
		timestep = std::clamp(timestep, 0.f, 0.1f); //!< Clamp to be no more than a 10th of a second for physics
		auto& pass = m_mainRenderer.getRenderPass(0);

		{
			/** Update scripts */
			auto view = m_mainScene->m_entities.view<ScriptComp>();
			for (auto entity : view) {
				auto& script = view.get<ScriptComp>(entity);
				script.onUpdate(timestep);
			}
		}
		


		/** Update camera and its position in UBO */
		auto& cameraTransform = m_mainScene->m_entities.get<TransformComponent>(camera);
		
		pass.camera.updateView(cameraTransform.transform);
		pass.setCachedValue("b_camera", "u_view", pass.camera.view);
		pass.setCachedValue("b_camera", "u_viewPos", cameraTransform.translation);
		m_cameraFrustum = CameraFrustum(pass.camera);

		/** Check for and action collisions */
		m_gameData.closeTargets.clear();
		m_gameData.closeAsteroids.clear();
		checkWaypointCollisions();
		checkAsteroidCollisions();

		/** Update skybox from camera */
		auto& skyboxRenderComp = m_mainScene->m_entities.get<RenderComponent>(skybox);
		skyboxRenderComp.material->setValue("u_skyboxView", glm::mat4(glm::mat3(pass.camera.view)));
	}
}

void AsteriodBelt::onImGUIRender()
{
	/** Scripts widgets */
	if (ImGui::TreeNode("Script settings"))
	{
		auto view = m_mainScene->m_entities.view<ScriptComp>();
		for (auto entity : view) {
			view.get<ScriptComp>(entity).onImGuiRender();
		}
		ImGui::TreePop();
	}
	ImGui::InputInt("Cull Count", &m_cullCount);

	/** Bloom detail */
	m_bloomPanel.onImGuiRender();
}

void AsteriodBelt::onKeyPressed(KeyPressedEvent& e)
{
	if (e.getKeyCode() == GLFW_KEY_SPACE && m_gameData.state == GameState::intro) m_gameData.state = GameState::running;

	if (e.getKeyCode() == GLFW_KEY_F)
	{
		if (m_wireframe) {
			m_wireframe = false;
			glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
			e.handle();
		}
		else {
			m_wireframe = true;
			glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
			e.handle();
		}
	}

	auto view = m_mainScene->m_entities.view<ScriptComp>();
	for (auto entity : view)
	{
		auto& script = view.get<ScriptComp>(entity);
		script.onKeyPress(e);
	}
}

void AsteriodBelt::checkWaypointCollisions()
{
	/** Generate collision point in front of ship */
	glm::vec3 offset(0.f, 0.18f, -0.14f);

	auto& shipTransform = m_mainScene->m_entities.get<TransformComponent>(ship);

	glm::vec3 shipRight = { -shipTransform.transform[0][0], -shipTransform.transform[0][1], -shipTransform.transform[0][2] };
	glm::vec3 shipUp = { -shipTransform.transform[1][0], -shipTransform.transform[1][1], -shipTransform.transform[1][2] };
	glm::vec3 shipForward = { shipTransform.transform[2][0], shipTransform.transform[2][1], shipTransform.transform[2][2] };

	glm::vec3 hitPoint = shipTransform.translation;

	hitPoint += shipRight * offset.x;
	hitPoint += shipUp * offset.y;
	hitPoint += shipForward * offset.z;

	/** Get all entities with an obb collider */
	auto view = m_mainScene->m_entities.view<OBBCollider, TransformComponent, Order>();

	m_cullCount = 0;
	for (auto& entity : view) {
		/** Compute distance */
		auto& obbCollider = view.get<OBBCollider>(entity);
		float dist = Physics::DistanceOBBToPoint(m_mainScene.get(), obbCollider, hitPoint);
		auto& renderComp = m_mainScene->m_entities.get<RenderComponent>(entity);

		if (m_cameraFrustum.intersects(obbCollider.aabb)) {
			renderComp.toCull = false;
		}
		else {
			renderComp.toCull = true;
			m_cullCount++;
		}

		if (dist < 0.75f) {
			/**
			 * Hit
			 * Make a record of order
			 */
			auto order = view.get<Order>(entity);

			/** Find the actor with the lowest order above the last order */
			entt::entity index{ entt::null };
			uint32_t lowest{ std::numeric_limits<uint32_t>::max() };

			auto orderView = m_mainScene->m_entities.view<Order>();
			for (auto& entity2 : orderView) {
				auto order2 = orderView.get<Order>(entity2);
				if (order2.order > order.order && order2.order < lowest) {
					index = entity2;
					lowest = order2.order;
				}
			}

			/** Give material to next target */
			if (index != entt::null) {
				/** Set material */
				auto& target = m_mainScene->m_entities.get<RenderComponent>(index);
				target.material = m_mainScene->m_entities.get<RenderComponent>(nextTarget).material;
				/** Update next target */
				nextTarget = index;
			}

			/** Remove waypoint that has been hit */
			m_mainScene->m_entities.destroy(entity);
		}

		if (dist < 35.f) {
			/** Ship to asteroid */
			glm::vec3 shipToTarget = view.get<TransformComponent>(entity).translation - m_mainScene->m_entities.get<TransformComponent>(ship).translation;
			/** Project to 2d for UI if infront of ship */
			if (glm::dot(shipToTarget, shipForward) > 0) {
				float x = glm::dot(shipRight, shipToTarget);
				float y = glm::dot(shipUp, shipToTarget);
				m_gameData.closeTargets.push_back(glm::vec3(x, y, 35.f - dist));
			}
		}
	}
}

void AsteriodBelt::checkAsteroidCollisions()
{
	/** Get forward, right and up vectors for projection */
	auto& shipTransform = m_mainScene->m_entities.get<TransformComponent>(ship);

	glm::vec3 shipRight = { -shipTransform.transform[0][0], -shipTransform.transform[0][1], -shipTransform.transform[0][2] };
	glm::vec3 shipUp = { -shipTransform.transform[1][0], -shipTransform.transform[1][1], -shipTransform.transform[1][2] };
	glm::vec3 shipForward = { shipTransform.transform[2][0], shipTransform.transform[2][1], shipTransform.transform[2][2] };

	/** Create a temporary OBB and AABB for the ship */
	glm::vec3 halfExtent = glm::vec3(0.72f, 0.18f, 1.f);
	OBBCollider obb(halfExtent, ship, shipTransform);

	/** Get all entities with a sphere collider */
	auto view = m_mainScene->m_entities.view<SphereCollider, TransformComponent>(); 

	m_cullCount = 0;
	for (auto& entity : view) {
		auto& sphereCollider = view.get<SphereCollider>(entity);
		auto& renderComp = m_mainScene->m_entities.get<RenderComponent>(entity);

		if (m_cameraFrustum.intersects(sphereCollider.aabb)) {
			renderComp.toCull = false;
		}
		else {
			renderComp.toCull = true;
			m_cullCount++;
		}

		float dist = Physics::DistanceOBBToSphere(m_mainScene.get(), obb, sphereCollider);

		if (dist < 25.f) {
			/** Ship to asteroid */
			glm::vec3 shipToAsteriod = view.get<TransformComponent>(entity).translation - m_mainScene->m_entities.get<TransformComponent>(ship).translation;
			/** Project to 2d for UI if infront of ship */
			if (glm::dot(shipToAsteriod, shipForward) > 0) {
				float x = glm::dot(shipRight, shipToAsteriod);
				float y = glm::dot(shipUp, shipToAsteriod);
				m_gameData.closeAsteroids.push_back(glm::vec3(x, y, 25.f - dist));
			}
		}

		if (dist < 0.f) m_gameData.state = GameState::gameOver;
	}
}

void AsteriodBelt::generateLevel() 
{
	const int wayPointCount = 100; //!< Number of waypoints
	const int asteroidsPerWayPointCount = 20; //!< Number of asteroids per way Point
	const float innerRadius = 20.f; //!< Closest the centre of an asteriod can be
	const float outerRadius = 80.f; //!< Furthest the centre of an asteriod can be

	m_mainScene.reset(new Scene);

	/** Skybox */
	ShaderDescription skyboxShaderDesc;
	skyboxShaderDesc.type = ShaderType::rasterization;
	skyboxShaderDesc.vertexSrcPath = "./assets/shaders/Skybox/Vert.glsl";
	skyboxShaderDesc.fragmentSrcPath = "./assets/shaders/Skybox/Frag.glsl";

	std::shared_ptr<Shader> skyboxShader;
	skyboxShader = std::make_shared<Shader>(skyboxShaderDesc);

	std::vector<float> skyboxVertices = {
		/** positions */
		-1000.f,  1000.f, -1000.f,
		-1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,
		1000.f,  1000.f, -1000.f,
		-1000.f,  1000.f, -1000.f,

		-1000.f, -1000.f,  1000.f,
		-1000.f, -1000.f, -1000.f,
		-1000.f,  1000.f, -1000.f,
		-1000.f,  1000.f, -1000.f,
		-1000.f,  1000.f,  1000.f,
		-1000.f, -1000.f,  1000.f,

		1000.f, -1000.f, -1000.f,
		1000.f, -1000.f,  1000.f,
		1000.f,  1000.f,  1000.f,
		1000.f,  1000.f,  1000.f,
		1000.f,  1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,

		-1000.f, -1000.f,  1000.f,
		-1000.f,  1000.f,  1000.f,
		1000.f,  1000.f,  1000.f,
		1000.f,  1000.f,  1000.f,
		1000.f, -1000.f,  1000.f,
		-1000.f, -1000.f,  1000.f,

		-1000.f,  1000.f, -1000.f,
		1000.f,  1000.f, -1000.f,
		1000.f,  1000.f,  1000.f,
		1000.f,  1000.f,  1000.f,
		-1000.f,  1000.f,  1000.f,
		-1000.f,  1000.f, -1000.f,

		-1000.f, -1000.f, -1000.f,
		-1000.f, -1000.f,  1000.f,
		1000.f, -1000.f, -1000.f,
		1000.f, -1000.f, -1000.f,
		-1000.f, -1000.f,  1000.f,
		1000.f, -1000.f,  1000.f
	};
	std::vector<uint32_t> skyboxIndices(skyboxVertices.size() / 3);
	std::iota(skyboxIndices.begin(), skyboxIndices.end(), 0);

	std::shared_ptr<VAO> skyboxVAO;
	skyboxVAO = std::make_shared<VAO>(skyboxIndices);
	skyboxVAO->addVertexBuffer(skyboxVertices, { {GL_FLOAT, 3} });

	std::array<const char*, 6> cubeMapPaths = {
		"./assets/textures/asteroidSkybox/right.png",
		"./assets/textures/asteroidSkybox/left.png",
		"./assets/textures/asteroidSkybox/top.png",
		"./assets/textures/asteroidSkybox/bottom.png",
		"./assets/textures/asteroidSkybox/front.png",
		"./assets/textures/asteroidSkybox/back.png"
	};

	std::shared_ptr<CubeMap> cubeMap;
	cubeMap = std::make_shared<CubeMap>(cubeMapPaths, false);

	std::shared_ptr<Material> skyboxMaterial;
	skyboxMaterial = std::make_shared<Material>(skyboxShader);
	skyboxMaterial->setValue("u_skybox", cubeMap);
	
	skybox = m_mainScene->m_entities.create();
	auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(skybox);
	renderComp.geometry = skyboxVAO;
	renderComp.material = skyboxMaterial;
	renderComp.LOD_data[0] = std::pair<uint32_t, uint32_t>(0, skyboxIndices.size());
	renderComp.LOD_data[1] = std::pair<uint32_t, uint32_t>(0, skyboxIndices.size());
	renderComp.LOD_data[2] = std::pair<uint32_t, uint32_t>(0, skyboxIndices.size());

	auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(skybox);
	skyboxMaterial->setValue("u_skyboxView", glm::inverse(transformComp.transform));	

	DirectionalLight dl;
	dl.direction = glm::normalize(glm::vec3(1.f, -3.5f, -2.f));
	m_mainScene->m_directionalLights.push_back(dl);
	
	camera = m_mainScene->m_entities.create();
	m_mainScene->m_entities.emplace<TransformComponent>(camera);

	ShaderDescription pbrShaderDesc;
	pbrShaderDesc.type = ShaderType::rasterization;
	pbrShaderDesc.vertexSrcPath = "./assets/shaders/PBR/pbrVertex.glsl";
	pbrShaderDesc.fragmentSrcPath = "./assets/shaders/PBR/pbrFrag.glsl";

	std::shared_ptr<Shader> pbrShader;
	pbrShader = std::make_shared<Shader>(pbrShaderDesc);

	ShaderDescription pbrEShaderDesc;
	pbrEShaderDesc.type = ShaderType::rasterization;
	pbrEShaderDesc.vertexSrcPath = "./assets/shaders/PBR/pbrVertex.glsl";
	pbrEShaderDesc.fragmentSrcPath = "./assets/shaders/PBR/pbrFragEmissive.glsl";

	std::shared_ptr<Shader> pbrEmissiveShader;
	pbrEmissiveShader = std::make_shared<Shader>(pbrEShaderDesc);

	VBOLayout modelLayout = {
				{GL_FLOAT, 3},
				{GL_FLOAT, 3},
				{GL_FLOAT, 2},
				{GL_FLOAT, 3}
	};
	VBOLayout depthLayout = { {GL_FLOAT, 3} };

	uint32_t attributeTypes = Model::VertexFlags::positions |
		Model::VertexFlags::normals |
		Model::VertexFlags::uvs |
		Model::VertexFlags::tangents;

	/** Ship */
	Model shipModel("./assets/models/ship/Spaceship.obj", attributeTypes, false);
	/** Body */
	std::shared_ptr<VAO> shipVAO;
	shipVAO = std::make_shared<VAO>(shipModel.m_meshes[0].indices);
	shipVAO->addVertexBuffer(shipModel.m_meshes[0].vertices, modelLayout);

	std::shared_ptr<Texture> ship_albedo = std::make_shared<Texture>("./assets/models/ship/albedo.jpg");
	std::shared_ptr<Texture> ship_normal = std::make_shared<Texture>("./assets/models/ship/normal.jpg");
	std::shared_ptr<Texture> ship_metal = std::make_shared<Texture>("./assets/models/ship/metallic.jpg");
	std::shared_ptr<Texture> ship_rough = std::make_shared<Texture>("./assets/models/ship/roughness.jpg");
	std::shared_ptr<Texture> ship_AO = std::make_shared<Texture>("./assets/models/ship/AO.png");

	std::shared_ptr<Material> shipMaterial;
	shipMaterial = std::make_shared<Material>(pbrShader, "u_model");
	shipMaterial->setValue("albedoTexture", ship_albedo);
	shipMaterial->setValue("normalTexture", ship_normal);
	shipMaterial->setValue("roughTexture", ship_rough);
	shipMaterial->setValue("metalTexture", ship_metal);
	shipMaterial->setValue("aoTexture", ship_AO);

	{
		ship = m_mainScene->m_entities.create();

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(ship);
		renderComp.geometry = shipVAO;
		renderComp.material = shipMaterial;
		renderComp.LOD_data[0] = std::pair<uint32_t, uint32_t>(0, shipModel.m_meshes[0].indices.size());
		renderComp.LOD_data[1] = std::pair<uint32_t, uint32_t>(0, shipModel.m_meshes[0].indices.size());
		renderComp.LOD_data[2] = std::pair<uint32_t, uint32_t>(0, shipModel.m_meshes[0].indices.size());

		auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(ship);
		transformComp.translation = glm::vec3(0.f, 0.f, -2.f);
		transformComp.rotation = glm::quat(glm::vec3(0.f, glm::pi<float>(), 0.f));
		transformComp.scale = glm::vec3(1.f);
		transformComp.recalc();

		auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(ship);
		/** Attach controller script */ 
		scriptComp.attachScript<ControllerScript>(ship, m_mainScene, m_winRef, camera, glm::vec3(0.06f, 0.06f, -1.5f), glm::vec3(0.f, 0.7f, 2.6f), &m_gameData.speed);
	}

	std::array<std::shared_ptr<VAO>, 4> asteroidVAOs;
	std::array<std::shared_ptr<Material>, 4> asteroidMaterials;
	std::array<LOD_data, 4> asteroidLODs;

	{
		/**	Asteroid 1 */
		Model asteroidModel("./assets/models/asteroid1/asteroid.obj", attributeTypes, true);
		asteroidVAOs[0] = std::make_shared<VAO>(asteroidModel.m_meshes[0].allIndices);
		asteroidVAOs[0]->addVertexBuffer(asteroidModel.m_meshes[0].vertices, modelLayout);

		asteroidLODs[0] = asteroidModel.m_meshes[0].getLODData();

		std::shared_ptr<Texture> asteroid_albedo = std::make_shared<Texture>("./assets/models/asteroid1/albedo.jpg");
		std::shared_ptr<Texture> asteroid_normal = std::make_shared<Texture>("./assets/models/asteroid1/normal.jpg");
		std::shared_ptr<Texture> asteroid_metal = std::make_shared<Texture>("./assets/models/asteroid1/metallic.jpg");
		std::shared_ptr<Texture> asteroid_rough = std::make_shared<Texture>("./assets/models/asteroid1/roughness.jpg");
		std::shared_ptr<Texture> asteroid_AO = std::make_shared<Texture>("./assets/models/asteroid1/AO.png");

		asteroidMaterials[0] = std::make_shared<Material>(pbrShader, "u_model");
		asteroidMaterials[0]->setValue("albedoTexture", asteroid_albedo);
		asteroidMaterials[0]->setValue("normalTexture", asteroid_normal);
		asteroidMaterials[0]->setValue("roughTexture", asteroid_rough);
		asteroidMaterials[0]->setValue("metalTexture", asteroid_metal);
		asteroidMaterials[0]->setValue("aoTexture", asteroid_AO);
	}

	{
		/**	Asteroid 2 */
		Model asteroidModel("./assets/models/asteroid2/asteroid.obj", attributeTypes, true);
		asteroidVAOs[1] = std::make_shared<VAO>(asteroidModel.m_meshes[0].allIndices);
		asteroidVAOs[1]->addVertexBuffer(asteroidModel.m_meshes[0].vertices, modelLayout);

		asteroidLODs[1] = asteroidModel.m_meshes[0].getLODData();

		std::shared_ptr<Texture> asteroid_albedo = std::make_shared<Texture>("./assets/models/asteroid2/albedo.jpg");
		std::shared_ptr<Texture> asteroid_normal = std::make_shared<Texture>("./assets/models/asteroid2/normal.jpg");
		std::shared_ptr<Texture> asteroid_metal = std::make_shared<Texture>("./assets/models/asteroid2/metallic.jpg");
		std::shared_ptr<Texture> asteroid_rough = std::make_shared<Texture>("./assets/models/asteroid2/roughness.jpg");
		std::shared_ptr<Texture> asteroid_AO = std::make_shared<Texture>("./assets/models/asteroid2/AO.png");

		asteroidMaterials[1] = std::make_shared<Material>(pbrShader, "u_model");
		asteroidMaterials[1]->setValue("albedoTexture", asteroid_albedo);
		asteroidMaterials[1]->setValue("normalTexture", asteroid_normal);
		asteroidMaterials[1]->setValue("roughTexture", asteroid_rough);
		asteroidMaterials[1]->setValue("metalTexture", asteroid_metal);
		asteroidMaterials[1]->setValue("aoTexture", asteroid_AO);
	}

	{
		/** Asteroid 3 */
		Model asteroidModel("./assets/models/asteroid3/asteroid.obj", attributeTypes, true);
		asteroidVAOs[2] = std::make_shared<VAO>(asteroidModel.m_meshes[0].allIndices);
		asteroidVAOs[2]->addVertexBuffer(asteroidModel.m_meshes[0].vertices, modelLayout);

		asteroidLODs[2] = asteroidModel.m_meshes[0].getLODData();

		std::shared_ptr<Texture> asteroid_albedo = std::make_shared<Texture>("./assets/models/asteroid3/albedo.jpg");
		std::shared_ptr<Texture> asteroid_normal = std::make_shared<Texture>("./assets/models/asteroid3/normal.jpg");
		std::shared_ptr<Texture> asteroid_metal = std::make_shared<Texture>("./assets/models/asteroid3/metallic.jpg");
		std::shared_ptr<Texture> asteroid_rough = std::make_shared<Texture>("./assets/models/asteroid3/roughness.jpg");
		std::shared_ptr<Texture> asteroid_AO = std::make_shared<Texture>("./assets/models/asteroid3/AO.png");

		asteroidMaterials[2] = std::make_shared<Material>(pbrShader, "u_model");
		asteroidMaterials[2]->setValue("albedoTexture", asteroid_albedo);
		asteroidMaterials[2]->setValue("normalTexture", asteroid_normal);
		asteroidMaterials[2]->setValue("roughTexture", asteroid_rough);
		asteroidMaterials[2]->setValue("metalTexture", asteroid_metal);
		asteroidMaterials[2]->setValue("aoTexture", asteroid_AO);
	}

	{
		/** Asteroid 4 */
		Model asteroidModel("./assets/models/asteroid4/asteroid.obj", attributeTypes, true);
		asteroidVAOs[3] = std::make_shared<VAO>(asteroidModel.m_meshes[0].allIndices);
		asteroidVAOs[3]->addVertexBuffer(asteroidModel.m_meshes[0].vertices, modelLayout);

		asteroidLODs[3] = asteroidModel.m_meshes[0].getLODData();

		std::shared_ptr<Texture> asteroid_albedo = std::make_shared<Texture>("./assets/models/asteroid4/albedo.jpg");
		std::shared_ptr<Texture> asteroid_normal = std::make_shared<Texture>("./assets/models/asteroid4/normal.jpg");
		std::shared_ptr<Texture> asteroid_metal = std::make_shared<Texture>("./assets/models/asteroid4/metallic.jpg");
		std::shared_ptr<Texture> asteroid_rough = std::make_shared<Texture>("./assets/models/asteroid4/roughness.jpg");
		std::shared_ptr<Texture> asteroid_AO = std::make_shared<Texture>("./assets/models/asteroid4/AO.png");

		asteroidMaterials[3] = std::make_shared<Material>(pbrShader, "u_model");
		asteroidMaterials[3]->setValue("albedoTexture", asteroid_albedo);
		asteroidMaterials[3]->setValue("normalTexture", asteroid_normal);
		asteroidMaterials[3]->setValue("roughTexture", asteroid_rough);
		asteroidMaterials[3]->setValue("metalTexture", asteroid_metal);
		asteroidMaterials[3]->setValue("aoTexture", asteroid_AO);
	}

	/** Waypoints */
	ShaderDescription phongEmissiveShdrDesc;
	phongEmissiveShdrDesc.type = ShaderType::rasterization;
	phongEmissiveShdrDesc.vertexSrcPath = "./assets/shaders/Phong/Vert.glsl";
	phongEmissiveShdrDesc.fragmentSrcPath = "./assets/shaders/Phong/EmissiveFrag.glsl";

	std::shared_ptr<Shader> phongEmissiveShader;
	phongEmissiveShader = std::make_shared<Shader>(phongEmissiveShdrDesc);

	VBOLayout cubeLayout = {
		{GL_FLOAT, 3},
		{GL_FLOAT, 3},
		{GL_FLOAT, 2}
	};

	Model cubeModel("./assets/models/whiteCube/letterCube.obj");

	std::shared_ptr<VAO> cubeVAO;
	cubeVAO = std::make_shared<VAO>(cubeModel.m_meshes[0].indices);
	cubeVAO->addVertexBuffer(cubeModel.m_meshes[0].vertices, cubeLayout);

	std::shared_ptr<Texture> cubeTexture;
	cubeTexture = std::make_shared<Texture>("./assets/models/whiteCube/letterCube.png");

	std::shared_ptr<Material> cubeMaterial;
	cubeMaterial = std::make_shared<Material>(phongEmissiveShader, "u_model");
	cubeMaterial->setValue("u_albedo", glm::vec3(0.722f, 0.251f, 0.871f));
	cubeMaterial->setValue("u_emissive", glm::vec4(0.722f, 0.251f, 0.871f, 4.75f));
	cubeMaterial->setValue("u_albedoMap", cubeTexture);

	std::shared_ptr<Material> firstCubeMaterial;
	firstCubeMaterial = std::make_shared<Material>(phongEmissiveShader, "u_model");
	firstCubeMaterial->setValue("u_albedo", glm::vec3(0.392f, 0.859f, 0.196f));
	firstCubeMaterial->setValue("u_emissive", glm::vec4(0.392f, 0.859f, 0.196f, 4.75f));
	firstCubeMaterial->setValue("u_albedoMap", cubeTexture);

	cube = m_mainScene->m_entities.create();
	m_mainScene->m_entities.emplace<TransformComponent>(cube);
	
	for (int i = 0; i < wayPointCount - 1; i++)
	{
		auto& transformComp = m_mainScene->m_entities.get<TransformComponent>(cube);
		glm::vec3 right = { -transformComp.transform[0][0], -transformComp.transform[0][1], -transformComp.transform[0][2] };
		glm::vec3 up = { -transformComp.transform[1][0], -transformComp.transform[1][1], -transformComp.transform[1][2] };
		glm::vec3 forward = { -transformComp.transform[2][0], -transformComp.transform[2][1], -transformComp.transform[2][2] };

		/** Save last cube for asteroid generation */
		entt::entity oldCube = cube;
		/** Create a new cube */
		cube = m_mainScene->m_entities.create();
		if (i == 0) nextTarget = cube;

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(cube);
		renderComp.geometry = cubeVAO;
		if (i == 0) renderComp.material = firstCubeMaterial ;
		else renderComp.material = cubeMaterial;
		// mesh.m_LOD_data[0] = std::pair<uint32_t, uint32_t>(0, mesh.m_indices.size());
		renderComp.LOD_data[0] = std::pair<uint32_t, uint32_t>(0, cubeModel.m_meshes[0].indices.size());
		renderComp.LOD_data[1] = std::pair<uint32_t, uint32_t>(0, cubeModel.m_meshes[0].indices.size());
		renderComp.LOD_data[2] = std::pair<uint32_t, uint32_t>(0, cubeModel.m_meshes[0].indices.size());

		auto& newTransformComp = m_mainScene->m_entities.emplace<TransformComponent>(cube);
		newTransformComp.scale = glm::vec3(0.5f);

		/** Generate next way point */
		auto& order = m_mainScene->m_entities.emplace<Order>(cube);
		order.order = i;

		float fwdDelta = Randomiser::uniformFloatBetween(25.f, 35.f);
		newTransformComp.translation = transformComp.translation + forward * fwdDelta;
		
		if (m_mainScene->m_pointLights.size() < 50) { // Cap point lights for performance
			PointLight newPL;
			newPL.colour = glm::vec3(Randomiser::uniformFloatBetween(0.0, 1.0), Randomiser::uniformFloatBetween(0.0, 1.0), Randomiser::uniformFloatBetween(0.0, 1.0));
			newPL.position = transformComp.translation + forward + fwdDelta;
			newPL.constants = glm::vec3(0.125f, 0.11f, 0.06f);
			m_mainScene->m_pointLights.push_back(newPL);
		}

		m_mainScene->m_entities.emplace<OBBCollider>(cube, newTransformComp.scale * 0.5f, cube, transformComp);

		glm::vec3 EulerAngle(Randomiser::uniformFloatBetween(-0.3f, 0.3f), Randomiser::uniformFloatBetween(-0.4f, 0.4f), 0.f);
		glm::quat angleDelta(EulerAngle);
		newTransformComp.rotation *= angleDelta;
		newTransformComp.recalc();
		//auto secondIdx = m_mainScene->m_actors.size(); // Index of the second waypoint
		//cube.setOBB(cube.scale * 0.5f, secondIdx);

		/**
		* Second attempt without proper projection but simple intersection check.
		* Asteroids between waypoints.
		*/
		std::vector<TransformComponent> asteroidsThisWaypoint;
		asteroidsThisWaypoint.reserve(asteroidsPerWayPointCount);
		for (int j = 0; j < asteroidsPerWayPointCount; j++)
		{
			float t{ 0.f }, radius{ 0.f }, theta{ 0.f }, scale{ 0.f };
			glm::vec3 position(0.f);
			const float distThresholdSquared{ 1.f };

			bool tooClose{ true };
			while (tooClose)
			{	/** Generate variable to place asteroid */

				/** Distance along waypoint */
				t = Randomiser::uniformFloatBetween(0.f, 1.f);
				/** Radius */
				radius = Randomiser::uniformFloatBetween(innerRadius, outerRadius);
				/** Theta */
				theta = Randomiser::uniformFloatBetween(-glm::pi<float>(), glm::pi<float>());
				/** Scale */
				scale = Randomiser::uniformFloatBetween(0.4f, 5.f);

				/** Position */
				position = m_mainScene->m_entities.get<TransformComponent>(oldCube).translation;
				position += (forward * fwdDelta) * t;
				position.x += cos(theta) * radius;
				position.y += sin(theta) * radius;

				/** Check closeness */
				tooClose = false;
				for (auto& other : asteroidsThisWaypoint)
				{
					float dist = glm::length(position - other.translation);
					if (dist < distThresholdSquared) {
						tooClose = true;
						break; //!< Exit loop, we need to regenerate asteroid position
					}
				}
			}

			asteroid = m_mainScene->m_entities.create();

			auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(asteroid);
			auto modelIdx = Randomiser::uniformIntBetween(0, 3);
			renderComp.geometry = asteroidVAOs[modelIdx];
			renderComp.material = asteroidMaterials[modelIdx];
			renderComp.LOD_data = asteroidLODs[modelIdx];

			auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(asteroid);
			transformComp.translation = position;
			transformComp.scale = glm::vec3(scale, scale, scale);
			transformComp.recalc();

			//auto idx = m_mainScene->m_actors.size();
			//asteroid.setEllipsoid(scale, idx);

			auto x = Randomiser::uniformFloatBetween(-1.f, 1.f);
			auto y = Randomiser::uniformFloatBetween(-1.f, 1.f);
			auto z = Randomiser::uniformFloatBetween(-1.f, 1.f);
			auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(asteroid);
			auto& sphereCollider = m_mainScene->m_entities.emplace<SphereCollider>(asteroid, scale, asteroid, transformComp);
			scriptComp.attachScript<RotationScript>(asteroid, m_mainScene, glm::vec3(x, y, z), 0);
		}
	}
};