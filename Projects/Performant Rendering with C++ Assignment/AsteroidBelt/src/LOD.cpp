#include "LOD.hpp"
#include "rotation.hpp"
#include "components/renderComponent.hpp"
#include "components/transformComponent.hpp"
#include "components/scriptComponent.hpp"
#include <meshoptimizer.h>
#include <rendering/renderPass.hpp>

LOD::LOD(GLFWWindowImpl& win) : Layer(win) {
	m_mainScene.reset(new Scene);

	DirectionalLight dl;
	dl.direction = glm::normalize(glm::vec3(1.f, -2.5f, -3.5f));
	m_mainScene->m_directionalLights.push_back(dl);

	ShaderDescription pbrShaderDesc;
	pbrShaderDesc.type = ShaderType::rasterization;
	pbrShaderDesc.vertexSrcPath = "./assets/shaders/PBR/pbrVertex.glsl";
	pbrShaderDesc.fragmentSrcPath = "./assets/shaders/PBR/pbrFrag.glsl";

	std::shared_ptr<Shader> pbrShader;
	pbrShader = std::make_shared<Shader>(pbrShaderDesc);

	VBOLayout modelLayout = {
				{GL_FLOAT, 3},
				{GL_FLOAT, 3},
				{GL_FLOAT, 2},
				{GL_FLOAT, 3}
	};

	uint32_t attributeTypes = Model::VertexFlags::positions |
		Model::VertexFlags::normals |
		Model::VertexFlags::uvs |
		Model::VertexFlags::tangents;

	std::array<std::shared_ptr<VAO>, 4> asteroidVAOs;
	std::shared_ptr<Material> asteroidMaterials;

	//	Asteroid Model
	std::shared_ptr<Texture> asteroid_albedo = std::make_shared<Texture>("./assets/models/asteroid1/albedo.jpg");
	std::shared_ptr<Texture> asteroid_normal = std::make_shared<Texture>("./assets/models/asteroid1/normal.jpg");
	std::shared_ptr<Texture> asteroid_metal = std::make_shared<Texture>("./assets/models/asteroid1/metallic.jpg");
	std::shared_ptr<Texture> asteroid_rough = std::make_shared<Texture>("./assets/models/asteroid1/roughness.jpg");
	std::shared_ptr<Texture> asteroid_AO = std::make_shared<Texture>("./assets/models/asteroid1/AO.png");

	asteroidMaterials = std::make_shared<Material>(pbrShader, "u_model");
	asteroidMaterials->setValue("albedoTexture", asteroid_albedo);
	asteroidMaterials->setValue("normalTexture", asteroid_normal);
	asteroidMaterials->setValue("roughTexture", asteroid_rough);
	asteroidMaterials->setValue("metalTexture", asteroid_metal);
	asteroidMaterials->setValue("aoTexture", asteroid_AO);

	/** Original Model Mesh */
	Model asteroidModel("./assets/models/asteroid1/asteroid.obj", attributeTypes);

	/** Mesh Optimisation */
	{
		const size_t vertexComponents = (3 + 3 + 2 + 3); //!< position, normal, uv, tangent
		const size_t vertexStride = sizeof(float) * vertexComponents;
		const size_t vertexCountOnLoad = asteroidModel.m_meshes[0].vertices.size() / vertexComponents;
		const size_t indexCountOnLoad = asteroidModel.m_meshes[0].indices.size();

		std::vector<uint32_t> remap(asteroidModel.m_meshes[0].indices.size());

		const size_t vertexCount = meshopt_generateVertexRemap(
			remap.data(),
			asteroidModel.m_meshes[0].indices.data(),
			indexCountOnLoad,
			asteroidModel.m_meshes[0].vertices.data(),
			vertexCountOnLoad,
			vertexStride
		);

		std::vector<uint32_t> remappedIndices(asteroidModel.m_meshes[0].indices.size());
		std::vector<float> remappedVertices(vertexCount * vertexComponents);

		meshopt_remapIndexBuffer(
			remappedIndices.data(),
			asteroidModel.m_meshes[0].indices.data(),
			indexCountOnLoad,
			remap.data());


		meshopt_remapVertexBuffer(
			remappedVertices.data(),
			asteroidModel.m_meshes[0].vertices.data(),
			vertexCountOnLoad,
			vertexStride,
			remap.data()
		);

		m_indices = remappedIndices;
		m_vertices = remappedVertices;

		spdlog::info("Verts on load: {} indices on load: {} ", vertexCountOnLoad, indexCountOnLoad);
		spdlog::info("Optimised verts: {} indices: {}", remappedVertices.size() / vertexComponents, remappedIndices.size());  

		meshopt_optimizeVertexCache(
			remappedIndices.data(),
			remappedIndices.data(),
			indexCountOnLoad,
			vertexCount
		);

		meshopt_optimizeOverdraw(
			remappedIndices.data(),
			remappedIndices.data(),
			indexCountOnLoad,
			remappedVertices.data(),
			vertexCount,
			vertexStride,
			1.05f); //!< Docs suggest 1.05 is generally a good threshold

		meshopt_optimizeVertexFetch(
			remappedVertices.data(),
			remappedIndices.data(),
			indexCountOnLoad,
			remappedVertices.data(),
			vertexCount,
			vertexStride
		);

		const float threshold1 = 0.5f;	// LOD1 threshold, the lower the number the fewer vertices
		const float threshold2 = 0.25f; // LOD2 threshold
		size_t LOD1_target_index_count = size_t(remappedIndices.size() * threshold1); // Ideal LOD1 index count
		size_t LOD2_target_index_count = size_t(remappedIndices.size() * threshold2); // Ideal LOD2 index count
		const float LOD1_target_error = 0.01f; // Error allowed, the higher the number greater deviation allowed
		const float LOD2_target_error = 0.02f; // Error allowed, the higher the number greater deviation allowed

		m_indicesLOD1.resize(remappedIndices.size());

		uint32_t LOD1IndexCount = meshopt_simplify(
			m_indicesLOD1.data(),
			remappedIndices.data(),
			remappedIndices.size(),
			remappedVertices.data(),
			vertexCount,
			vertexStride,
			LOD1_target_index_count,
			LOD1_target_error
		);

		m_indicesLOD2.resize(remappedIndices.size());

		uint32_t LOD2IndexCount = meshopt_simplify(
			m_indicesLOD2.data(),
			remappedIndices.data(),
			remappedIndices.size(),
			remappedVertices.data(),
			vertexCount,
			vertexStride,
			LOD2_target_index_count,
			LOD2_target_error
		);

		m_indices = remappedIndices;
		m_vertices = remappedVertices;
		m_indicesLOD1.erase(m_indicesLOD1.begin() + LOD1IndexCount, m_indicesLOD1.end()); // Cut down to actual size
		m_indicesLOD2.erase(m_indicesLOD2.begin() + LOD2IndexCount, m_indicesLOD2.end()); // Cut down to actual size

		spdlog::info("Verts on load: {} indices on load: {} ", vertexCountOnLoad, indexCountOnLoad);
		spdlog::info("Optimised verts: {} indices: {}", remappedVertices.size() / vertexComponents, remappedIndices.size());
		spdlog::info("LOD1 target index count {}  actual index count {}", LOD1_target_index_count, LOD1IndexCount);
		spdlog::info("LOD2 target index count {}  actual index count {}", LOD2_target_index_count, LOD2IndexCount);
	}

	/** Concatonate indices */
	std::vector<uint32_t> allIndices;
	allIndices.insert(allIndices.end(), m_indices.begin(), m_indices.end());
	allIndices.insert(allIndices.end(), m_indicesLOD1.begin(), m_indicesLOD1.end());
	allIndices.insert(allIndices.end(), m_indicesLOD2.begin(), m_indicesLOD2.end());

	/** Create a VAO with all indices in */
	std::shared_ptr<VAO> allLODsVAO = std::make_shared<VAO>(allIndices);
	allLODsVAO->addVertexBuffer(m_vertices, modelLayout);

	{
		allLODs = m_mainScene->m_entities.create();

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(allLODs);
		renderComp.geometry = allLODsVAO;
		renderComp.material = asteroidMaterials;

		auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(allLODs);
		transformComp.translation = glm::vec3(0.f, 0.f, -6.f);
		transformComp.scale = glm::vec3(1.0);
		transformComp.recalc();

		auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(allLODs);
		scriptComp.attachScript<RotationScript>(allLODs, m_mainScene, glm::vec3(0.4f, 0.25f, -0.6f), GLFW_KEY_SPACE);
	}

	/** Set LOD data */
	LOD_data[0] = std::pair<uint32_t, uint32_t>(0, m_indices.size());
	LOD_data[1] = std::pair<uint32_t, uint32_t>(m_indices.size(), m_indicesLOD1.size());
	LOD_data[2] = std::pair<uint32_t, uint32_t>(m_indices.size() + m_indicesLOD1.size(), m_indicesLOD2.size());

	asteroidVAOs[0] = std::make_shared<VAO>(asteroidModel.m_meshes[0].indices);
	asteroidVAOs[0]->addVertexBuffer(asteroidModel.m_meshes[0].vertices, modelLayout);

	{
		entt::entity raw = m_mainScene->m_entities.create();

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(raw);
		renderComp.geometry = asteroidVAOs[0];
		renderComp.material = asteroidMaterials;

		auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(raw);
		transformComp.translation = glm::vec3(-2.5f, 1.65f, -5.f);
		transformComp.scale = glm::vec3(1.0);
		transformComp.recalc();

		auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(raw);
		scriptComp.attachScript<RotationScript>(raw, m_mainScene, glm::vec3(0.f), GLFW_KEY_SPACE);
	}

	/** Optimised Model Mesh */
	asteroidVAOs[1] = std::make_shared<VAO>(m_indices);
	asteroidVAOs[1]->addVertexBuffer(m_vertices, modelLayout);

	{
		entt::entity optimised = m_mainScene->m_entities.create();

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(optimised);
		renderComp.geometry = asteroidVAOs[1];
		renderComp.material = asteroidMaterials;

		auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(optimised);
		transformComp.translation = glm::vec3(2.5f, 1.65f, -5.f);
		transformComp.scale = glm::vec3(1.0);
		transformComp.recalc();

		auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(optimised);
		scriptComp.attachScript<RotationScript>(optimised, m_mainScene, glm::vec3(0.f), GLFW_KEY_SPACE);
	}

	/** LOD1 Model Mesh */
	asteroidVAOs[2] = std::make_shared<VAO>(m_indicesLOD1);
	asteroidVAOs[2]->addVertexBuffer(m_vertices, modelLayout);
	{
		entt::entity LOD1 = m_mainScene->m_entities.create();

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(LOD1);
		renderComp.geometry = asteroidVAOs[2];
		renderComp.material = asteroidMaterials;

		auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(LOD1);
		transformComp.translation = glm::vec3(-2.5f, -1.65f, -5.f);
		transformComp.scale = glm::vec3(1.0);
		transformComp.recalc();

		auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(LOD1);
		scriptComp.attachScript<RotationScript>(LOD1, m_mainScene, glm::vec3(0.4f, 0.25f, -0.6f), GLFW_KEY_SPACE);
	}

	/** LOD2 Model Mesh */
	asteroidVAOs[3] = std::make_shared<VAO>(m_indicesLOD2);
	asteroidVAOs[3]->addVertexBuffer(m_vertices, modelLayout);
	{
		entt::entity LOD2 = m_mainScene->m_entities.create();

		auto& renderComp = m_mainScene->m_entities.emplace<RenderComponent>(LOD2);
		renderComp.geometry = asteroidVAOs[3];
		renderComp.material = asteroidMaterials;

		auto& transformComp = m_mainScene->m_entities.emplace<TransformComponent>(LOD2);
		transformComp.translation = glm::vec3(2.5f, -1.65f, -5.f);
		transformComp.scale = glm::vec3(1.0);
		transformComp.recalc();

		auto& scriptComp = m_mainScene->m_entities.emplace<ScriptComp>(LOD2);
		scriptComp.attachScript<RotationScript>(LOD2, m_mainScene, glm::vec3(0.4f, 0.25f, -0.6f), GLFW_KEY_SPACE);
	}

	m_mainPass.scene = m_mainScene;
	m_mainPass.parseScene();
	m_mainPass.target = std::make_shared<FBO>();
	m_mainPass.camera.projection = glm::perspective(45.f, m_winRef.getWidthf() / m_winRef.getHeightf(), 0.1f, 2000.f);
	m_mainPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	m_mainPass.setCachedValue("b_camera", "u_view", m_mainPass.camera.view);
	m_mainPass.setCachedValue("b_camera", "u_projection", m_mainPass.camera.projection);
	m_mainPass.setCachedValue("b_camera", "u_viewPos", glm::vec3(0.f));

	m_mainPass.setCachedValue("b_lights", "dLight.colour", m_mainScene->m_directionalLights.at(0).colour);
	m_mainPass.setCachedValue("b_lights", "dLight.direction", m_mainScene->m_directionalLights.at(0).direction);

	m_mainRenderer.addRenderPass(m_mainPass);

	glClearColor(1.f, 0.f, 1.f, 1.f);
}

void LOD::onRender() const {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	m_mainPass.uploadCachedValues();

	auto& renderComp = m_mainScene->m_entities.get<RenderComponent>(allLODs);
	auto& transformComp = m_mainScene->m_entities.get<TransformComponent>(allLODs);

	// Apply material
	renderComp.material->apply();
	renderComp.material->m_shader->uploadUniform(renderComp.material->getTransformUniformName(), transformComp.transform);

	// Bind VAO
	glBindVertexArray(renderComp.geometry->getID());

	// Draw LOD
	void* baseVertexIndex = (void*)(sizeof(GLuint) * LOD_data[LODidx].first);
	auto& drawCount = LOD_data[LODidx].second;
	glDrawElements(renderComp.material->getPrimitive(), drawCount, GL_UNSIGNED_INT, baseVertexIndex);
}

void LOD::onUpdate(float timestep) {
	timestep = std::clamp(timestep, 0.f, 0.1f); // Clamp to be no more than a 10th of a second for physics
	// Update scripts
	auto view = m_mainScene->m_entities.view<ScriptComp>();
	for (auto entity : view)
	{
		auto& script = view.get<ScriptComp>(entity);
		script.onUpdate(timestep);
	}
}

void LOD::onKeyPressed(KeyPressedEvent& e) {
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

	// scripts
	auto view = m_mainScene->m_entities.view<ScriptComp>();
	for (auto entity : view)
	{
		auto& script = view.get<ScriptComp>(entity);
		script.onKeyPress(e);
	}
}

void LOD::onImGUIRender() {
	// Scripts
	auto view = m_mainScene->m_entities.view<ScriptComp>();
	for (auto entity : view)
	{
		auto& script = view.get<ScriptComp>(entity);
		script.onImGuiRender();
	}
	static const char* items[]{ "None", "One","Two" };
	static int Selecteditem = 0;
	if (ImGui::Combo("LOD", &Selecteditem, items, IM_ARRAYSIZE(items)))
	{
		LODidx = Selecteditem;
	}
}
