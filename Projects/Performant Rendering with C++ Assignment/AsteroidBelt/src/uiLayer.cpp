/** @file uiLayer.cpp */
#include "uiLayer.hpp"
#include <components/renderComponent.hpp>

UI::UI(GLFWWindowImpl& win, SharedData& data) : Layer(win), m_gameData(data)
{
	screenLODs[0] = { 0, 6 };
	screenLODs[1] = { 0, 6 };
	screenLODs[2] = { 0, 6 };

	m_UIScene.reset(new Scene);

	/** Textures */
	m_introTexture.reset(new Texture("./assets/textures/UI/intro.png"));
	m_gameOverTexture.reset(new Texture("./assets/textures/UI/gameOver.png"));

	/** Quad shader - implements programmable vertex pulling */
	ShaderDescription quadShaderDesc;
	quadShaderDesc.type = ShaderType::rasterization;
	quadShaderDesc.vertexSrcPath = "./assets/shaders/UI/QuadVert.glsl";
	quadShaderDesc.fragmentSrcPath = "./assets/shaders/UI/QuadFrag.glsl";

	std::shared_ptr<Shader> quadShader;
	quadShader = std::make_shared<Shader>(quadShaderDesc);

	/** Circle shader - implements programmable vertex pulling */
	ShaderDescription circleShaderDesc;
	circleShaderDesc.type = ShaderType::rasterization;
	circleShaderDesc.vertexSrcPath = "./assets/shaders/UI/CircleVert.glsl";
	circleShaderDesc.fragmentSrcPath = "./assets/shaders/UI/CircleFrag.glsl";

	std::shared_ptr<Shader> circleShader;
	circleShader = std::make_shared<Shader>(circleShaderDesc);

	/** Populate indices - common to all */
	std::vector<uint32_t> indices(m_maxQuadCount * 6);

	uint32_t offset{ 0 };
	for (size_t i = 0; i < indices.size(); i += 6)
	{
		indices[i + 0] = offset + 0;
		indices[i + 1] = offset + 1;
		indices[i + 2] = offset + 2;

		indices[i + 3] = offset + 2;
		indices[i + 4] = offset + 3;
		indices[i + 5] = offset + 0;

		offset += 4;
	}

	/** Quads - Set up buffers, material and actor */
	std::shared_ptr<VAO> quadsVAO;
	quadsVAO = std::make_shared<VAO>(indices);

	m_quadsSSBO = std::make_shared<SSBO>(sizeof(QuadVertex) * m_quadVertices.size(), m_quadVertices.size());
	m_quadsSSBO->edit(0, sizeof(QuadVertex) * m_quadVertices.size(), m_quadVertices.data());

	std::shared_ptr<Material> quadsMaterial;
	quadsMaterial = std::make_shared<Material>(quadShader, "");

	{
		m_quads = m_UIScene->m_entities.create();

		m_UIScene->m_entities.emplace<TransformComponent>(m_quads);

		auto& renderComp = m_UIScene->m_entities.emplace<RenderComponent>(m_quads);
		renderComp.geometry = quadsVAO;
		renderComp.material = quadsMaterial;
		renderComp.LOD_data[0] = std::pair<uint32_t, uint32_t>(0, indices.size());
		renderComp.LOD_data[1] = std::pair<uint32_t, uint32_t>(0, indices.size());
		renderComp.LOD_data[2] = std::pair<uint32_t, uint32_t>(0, indices.size());

		renderComp.material->setValue("u_textureSlots[0]", m_slots.data());
		renderComp.geometry->overrideDrawCount(0);
	}

	/** Circles - Set up buffers, material and actor */
	std::shared_ptr<VAO> circlesVAO;
	circlesVAO = std::make_shared<VAO>(indices); //!< Same as quads indices - circles are beounded by quads

	m_circlesSSBO = std::make_shared<SSBO>(sizeof(CircleVertex) * m_circleVertices.size(), m_circleVertices.size());
	m_circlesSSBO->edit(0, sizeof(CircleVertex) * m_circleVertices.size(), m_circleVertices.data());

	std::shared_ptr<Material> circleMaterial;
	circleMaterial = std::make_shared<Material>(circleShader, "");

	{
		m_circles = m_UIScene->m_entities.create();

		m_UIScene->m_entities.emplace<TransformComponent>(m_circles);

		auto& renderComp = m_UIScene->m_entities.emplace<RenderComponent>(m_circles);
		renderComp.geometry = circlesVAO;
		renderComp.material = circleMaterial;
		renderComp.LOD_data[0] = std::pair<uint32_t, uint32_t>(0, indices.size());
		renderComp.LOD_data[1] = std::pair<uint32_t, uint32_t>(0, indices.size());
		renderComp.LOD_data[2] = std::pair<uint32_t, uint32_t>(0, indices.size());

		renderComp.geometry->overrideDrawCount(0);
	}

	/** Clear quad data */
	m_currentQuadCount = 0;
	auto& quadsRender = m_UIScene->m_entities.get<RenderComponent>(m_quads);
	quadsRender.geometry->overrideDrawCount(0);

	/** Clear circle data */
	m_currentCircleCount = 0;
	auto& circlesRender = m_UIScene->m_entities.get<RenderComponent>(m_circles);
	circlesRender.geometry->overrideDrawCount(0);

	/*************************
	*  UI Pass
	**************************/
	RenderPass UIpass;
	UIpass.scene = m_UIScene;
	UIpass.parseScene();
	UIpass.clearColour = false;
	UIpass.clearDepth = false;
	UIpass.target = std::make_shared<FBO>(); //!< Default FBO
	UIpass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };
	UIpass.camera.view = glm::mat4(1.f);
	UIpass.camera.projection = glm::ortho(0.f, m_winRef.getWidthf(), m_winRef.getHeightf(), 0.f);
	UIpass.setCachedValue("b_cameraUI", "u_view", UIpass.camera.view);
	UIpass.setCachedValue("b_cameraUI", "u_projection", UIpass.camera.projection);

	m_UIRenderer.addRenderPass(UIpass);

	/** Setup default texture and sub - texture */
	unsigned char whitePx[4] = { 255,255,255,255 };
	// RendererCommon::defaultTexture = resources->addTexture("defaultWhite", 1, 1, 4, whitePx);
	TextureDescription defaultTexDesc;
	defaultTexDesc.channels = 4;
	defaultTexDesc.width = 1;
	defaultTexDesc.height = 1;
	defaultTexDesc.type = TextureDataType::SDR;

	m_defaultTexture = std::make_shared<Texture>(defaultTexDesc, whitePx);
	// RendererCommon::defaultSubTexture = SubTexture(RendererCommon::defaultTexture, glm::vec2(0.f, 0.f), glm::vec2(1.f, 1.f));

	m_quadsSSBO->bind(0);
	m_circlesSSBO->bind(1);
}

void UI::onRender() const
{
	/** Clear all texture slots */
	m_defaultTexture->clearTUM();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_UIRenderer.render();

	glDisable(GL_BLEND);
}

void UI::onUpdate(float timestep)
{
	begin();
	if (m_gameData.state == GameState::intro) drawTexturedQuad({ 0.f, 0.f }, m_winRef.getSizef(), m_introTexture);
	if (m_gameData.state == GameState::gameOver) drawTexturedQuad({ 0.f, 0.f }, m_winRef.getSizef(), m_gameOverTexture);
	if (m_gameData.state == GameState::running) {
		/** m_gameData.speed widget */
		glm::vec4 noColor(0.58, 0.573, 0.678, 0.60);
		/** Quads */
		drawQuad({ 1015.23429376605f, 677.060052230405f }, { 15.f, 50.9399477695946f }, m_speedUIColours[0]);
		drawQuad({ 1036.10357597866f, 676.183738511636f }, { 15.f, 51.8162614883643f }, m_gameData.speed < m_speedThresholds[0] ? m_speedUIColours[1] : noColor);
		drawQuad({ 1056.97285819127f, 675.142292174748f }, { 15.f, 52.8577078252518f }, m_gameData.speed < m_speedThresholds[1] ? m_speedUIColours[2] : noColor);
		drawQuad({ 1077.84214040388f, 673.934247237929f }, { 15.f, 54.065752762071f }, m_gameData.speed < m_speedThresholds[2] ? m_speedUIColours[3] : noColor);
		drawQuad({ 1098.71142261648f, 672.557881377036f }, { 15.f, 55.4421186229637f }, m_gameData.speed < m_speedThresholds[3] ? m_speedUIColours[4] : noColor);
		drawQuad({ 1119.58070482909f, 671.011203218924f }, { 15.f, 56.9887967810764f }, m_gameData.speed < m_speedThresholds[4] ? m_speedUIColours[5] : noColor);
		drawQuad({ 1140.4499870417f,  669.291937293714f }, { 15.f, 58.7080627062856f }, m_gameData.speed < m_speedThresholds[5] ? m_speedUIColours[6] : noColor);
		drawQuad({ 1161.31926925431f, 667.397506369484f }, { 15.f, 60.602493630516f }, m_gameData.speed < m_speedThresholds[6] ? m_speedUIColours[7] : noColor);
		drawQuad({ 1182.18855146691f, 665.325010832928f }, { 15.f, 62.6749891670719f }, m_gameData.speed < m_speedThresholds[7] ? m_speedUIColours[8] : noColor);
		drawQuad({ 1203.05783367952f, 663.071204707666f }, { 15.f, 64.9287952923339f }, m_gameData.speed < m_speedThresholds[8] ? m_speedUIColours[9] : noColor);
		drawQuad({ 1223.92711589213f, 660.632467814566f }, { 15.f, 67.3675321854339f }, m_gameData.speed < m_speedThresholds[9] ? m_speedUIColours[10] : noColor);
		drawQuad({ 1244.79639810473f, 658.004773471693f }, { 15.f, 69.9952265283073f }, m_gameData.speed < m_speedThresholds[10] ? m_speedUIColours[11] : noColor);
		drawQuad({ 1265.66568031734f, 655.183650999685f }, { 15.f, 72.8163490003149f }, m_gameData.speed < m_speedThresholds[11] ? m_speedUIColours[12] : noColor);
		drawQuad({ 1286.53496252995f, 652.164142134382f }, { 15.f, 75.8358578656179f }, m_gameData.speed < m_speedThresholds[12] ? m_speedUIColours[13] : noColor);
		drawQuad({ 1307.40424474256f, 648.940750242794f }, { 15.f, 79.0592497572063f }, m_gameData.speed < m_speedThresholds[13] ? m_speedUIColours[14] : noColor);
		drawQuad({ 1328.27352695516f, 645.50738097831f }, { 15.f, 82.4926190216903f }, m_gameData.speed < m_speedThresholds[14] ? m_speedUIColours[15] : noColor);

		/** Bottom Circles */
		drawCircle({ 1022.73429376605f, 728.f }, { 7.5f }, m_speedUIColours[0], 1.f);
		drawCircle({ 1043.60357597866f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[0] ? m_speedUIColours[1] : noColor, 1.f);
		drawCircle({ 1064.47285819127f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[1] ? m_speedUIColours[2] : noColor, 1.f);
		drawCircle({ 1085.34214040388f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[2] ? m_speedUIColours[3] : noColor, 1.f);
		drawCircle({ 1106.21142261648f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[3] ? m_speedUIColours[4] : noColor, 1.f);
		drawCircle({ 1127.08070482909f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[4] ? m_speedUIColours[5] : noColor, 1.f);
		drawCircle({ 1147.9499870417f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[5] ? m_speedUIColours[6] : noColor, 1.f);
		drawCircle({ 1168.81926925431f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[6] ? m_speedUIColours[7] : noColor, 1.f);
		drawCircle({ 1189.68855146691f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[7] ? m_speedUIColours[8] : noColor, 1.f);
		drawCircle({ 1210.55783367952f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[8] ? m_speedUIColours[9] : noColor, 1.f);
		drawCircle({ 1231.42711589213f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[9] ? m_speedUIColours[10] : noColor, 1.f);
		drawCircle({ 1252.29639810473f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[10] ? m_speedUIColours[11] : noColor, 1.f);
		drawCircle({ 1273.16568031734f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[11] ? m_speedUIColours[12] : noColor, 1.f);
		drawCircle({ 1294.03496252995f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[12] ? m_speedUIColours[13] : noColor, 1.f);
		drawCircle({ 1314.90424474256f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[13] ? m_speedUIColours[14] : noColor, 1.f);
		drawCircle({ 1335.77352695516f, 728.f }, { 7.5f }, m_gameData.speed < m_speedThresholds[14] ? m_speedUIColours[15] : noColor, 1.f);

		/** Top circles */
		drawCircle({ 1022.73429376605f, 677.060052230405f }, { 7.5f }, m_speedUIColours[0], 1.f);
		drawCircle({ 1043.60357597866f, 676.183738511636f }, { 7.5f }, m_gameData.speed < m_speedThresholds[0] ? m_speedUIColours[1] : noColor, 1.f);
		drawCircle({ 1064.47285819127f, 675.142292174748f }, { 7.5f }, m_gameData.speed < m_speedThresholds[1] ? m_speedUIColours[2] : noColor, 1.f);
		drawCircle({ 1085.34214040388f, 673.934247237929f }, { 7.5f }, m_gameData.speed < m_speedThresholds[2] ? m_speedUIColours[3] : noColor, 1.f);
		drawCircle({ 1106.21142261648f, 672.557881377036f }, { 7.5f }, m_gameData.speed < m_speedThresholds[3] ? m_speedUIColours[4] : noColor, 1.f);
		drawCircle({ 1127.08070482909f, 671.011203218924f }, { 7.5f }, m_gameData.speed < m_speedThresholds[4] ? m_speedUIColours[5] : noColor, 1.f);
		drawCircle({ 1147.9499870417f,  669.291937293714f }, { 7.5f }, m_gameData.speed < m_speedThresholds[5] ? m_speedUIColours[6] : noColor, 1.f);
		drawCircle({ 1168.81926925431f, 667.397506369484f }, { 7.5f }, m_gameData.speed < m_speedThresholds[6] ? m_speedUIColours[7] : noColor, 1.f);
		drawCircle({ 1189.68855146691f, 665.325010832928f }, { 7.5f }, m_gameData.speed < m_speedThresholds[7] ? m_speedUIColours[8] : noColor, 1.f);
		drawCircle({ 1210.55783367952f, 663.071204707666f }, { 7.5f }, m_gameData.speed < m_speedThresholds[8] ? m_speedUIColours[9] : noColor, 1.f);
		drawCircle({ 1231.42711589213f, 660.632467814566f }, { 7.5f }, m_gameData.speed < m_speedThresholds[9] ? m_speedUIColours[10] : noColor, 1.f);
		drawCircle({ 1252.29639810473f, 658.004773471693f }, { 7.5f }, m_gameData.speed < m_speedThresholds[10] ? m_speedUIColours[11] : noColor, 1.f);
		drawCircle({ 1273.16568031734f, 655.183650999685f }, { 7.5f }, m_gameData.speed < m_speedThresholds[11] ? m_speedUIColours[12] : noColor, 1.f);
		drawCircle({ 1294.03496252995f, 652.164142134382f }, { 7.5f }, m_gameData.speed < m_speedThresholds[12] ? m_speedUIColours[13] : noColor, 1.f);
		drawCircle({ 1314.90424474256f, 648.940750242794f }, { 7.5f }, m_gameData.speed < m_speedThresholds[13] ? m_speedUIColours[14] : noColor, 1.f);
		drawCircle({ 1335.77352695516f, 645.50738097831f }, { 7.5f }, m_gameData.speed < m_speedThresholds[14] ? m_speedUIColours[15] : noColor, 1.f);


		/** Target circles */
		glm::vec4 targetCircleColour(0.909803921568627f, 0.f, 0.f, 0.65f);
		drawCircle({ 184.f, 618.f }, { 50.f }, targetCircleColour, 0.075f);
		drawCircle({ 184.f, 618.f }, { 75.f }, targetCircleColour, 0.0375f);
		drawCircle({ 184.f, 618.f }, { 100.f }, targetCircleColour, 0.01875f);
		drawCircle({ 184.f, 618.f }, { 125.f }, targetCircleColour, 0.009375f);

		
		/** Close Ship Circles */ 
		for (auto& circle : m_gameData.closeAsteroids) {
			float radius = 7.5f + circle.z / 2.5f;
			drawCircle({ 184.f + circle.x * 5.0f, 618.f + circle.y * 5.f }, { radius }, { targetCircleColour.x, targetCircleColour.y, targetCircleColour.z, circle.z / 50.f + 0.25f }, 0.2f);
		}
		
		/** Close Target quads */
		for (auto& quad : m_gameData.closeTargets) {
			float size = 10.f + quad.z / 3.5f;
			drawQuad({ 184.f + quad.x * 5.0f - size / 2.f, 618.f + quad.y * 5.f - size / 2.f }, { size, size }, { 0.463f, 0.031f, 0.769f, quad.z / 35.f });
		}
	}

	end();
}

void UI::begin()
{
	/** Clear quad data */
	m_currentQuadCount = 0;
	auto& quadsRender = m_UIScene->m_entities.get<RenderComponent>(m_quads);
	quadsRender.geometry->overrideDrawCount(0);

	/** Clear circle data */
	m_currentCircleCount = 0;
	auto& circlesRender = m_UIScene->m_entities.get<RenderComponent>(m_circles);
	circlesRender.geometry->overrideDrawCount(0);
}

void UI::end()
{
	/** Send quad vertices to the SSBO */ 
	m_quadsSSBO->edit(0, sizeof(QuadVertex) * m_currentQuadCount * 4, m_quadVertices.data());
	/** Set quad draw count */
	auto& quadsRender = m_UIScene->m_entities.get<RenderComponent>(m_quads);
	quadsRender.geometry->overrideDrawCount(m_currentQuadCount * 6);

	/** Send circle vertices to the SSBO */
	m_circlesSSBO->edit(0, sizeof(CircleVertex) * m_currentCircleCount * 4, m_circleVertices.data());
	/** Set circle draw count */
	auto& circlesRender = m_UIScene->m_entities.get<RenderComponent>(m_circles);
	circlesRender.geometry->overrideDrawCount(m_currentCircleCount * 6);

	/** Make sure texture slots are correct */
	quadsRender.material->setValue("u_textureSlots[0]", m_slots.data());
}

void UI::drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& colour)
{
	auto slot = m_defaultTexture->getUnit(m_defaultTexture->getID());

	size_t idx = m_currentQuadCount * 4;

	/** Position */
	m_quadVertices[idx + 0].position = { position.x, position.y }; 
	m_quadVertices[idx + 1].position = { position.x, position.y + size.y}; 
	m_quadVertices[idx + 2].position = { position.x + size.x, position.y + size.y };
	m_quadVertices[idx + 3].position = { position.x + size.x, position.y };
	/** UVs */
	m_quadVertices[idx + 0].texCoords = { 0.f, 0.f };
	m_quadVertices[idx + 1].texCoords = { 0.f, 1.f };
	m_quadVertices[idx + 2].texCoords = { 1.f, 1.f };
	m_quadVertices[idx + 3].texCoords = { 1.f, 0.f };
	/** Colour */
	m_quadVertices[idx + 0].colour = colour;
	m_quadVertices[idx + 1].colour = colour;
	m_quadVertices[idx + 2].colour = colour;
	m_quadVertices[idx + 3].colour = colour;
	/** Slot */
	m_quadVertices[idx + 0].texSlot = slot;
	m_quadVertices[idx + 1].texSlot = slot;
	m_quadVertices[idx + 2].texSlot = slot;
	m_quadVertices[idx + 3].texSlot = slot;

	m_currentQuadCount++;

	if (idx + 3 >= m_quadVertices.size()) spdlog::error("UI Quad vertices have reached limit");
}

void UI::drawTexturedQuad(const glm::vec2& position, const glm::vec2& size, std::shared_ptr<Texture> texture)
{
	auto slot = texture->getUnit(texture->getID());

	size_t idx = m_currentQuadCount * 4;

	/** Position */
	m_quadVertices[idx + 0].position = { position.x, position.y };
	m_quadVertices[idx + 1].position = { position.x, position.y + size.y };
	m_quadVertices[idx + 2].position = { position.x + size.x, position.y + size.y };
	m_quadVertices[idx + 3].position = { position.x + size.x, position.y };
	/** UVs */
	m_quadVertices[idx + 0].texCoords = { 0.f, 1.f };
	m_quadVertices[idx + 1].texCoords = { 0.f, 0.f };
	m_quadVertices[idx + 2].texCoords = { 1.f, 0.f };
	m_quadVertices[idx + 3].texCoords = { 1.f, 1.f };
	/** Colour */
	m_quadVertices[idx + 0].colour = m_defaultColour;
	m_quadVertices[idx + 1].colour = m_defaultColour;
	m_quadVertices[idx + 2].colour = m_defaultColour;
	m_quadVertices[idx + 3].colour = m_defaultColour;
	/** Slot */
	m_quadVertices[idx + 0].texSlot = slot;
	m_quadVertices[idx + 1].texSlot = slot;
	m_quadVertices[idx + 2].texSlot = slot;
	m_quadVertices[idx + 3].texSlot = slot;

	m_currentQuadCount++;

	if (idx + 3 >= m_quadVertices.size()) spdlog::error("UI Quad vertices have reached limit");
}

void UI::drawCircle(const glm::vec2& centre, float radius, const glm::vec4& colour, float thickness)
{

	size_t idx = m_currentCircleCount * 4;

	/** Position */
	m_circleVertices[idx + 0].position = { centre.x - radius, centre.y - radius};
	m_circleVertices[idx + 1].position = { centre.x - radius, centre.y + radius };
	m_circleVertices[idx + 2].position = { centre.x + radius, centre.y + radius };
	m_circleVertices[idx + 3].position = { centre.x + radius, centre.y - radius };
	/** Unit position */
	m_circleVertices[idx + 0].unitPosition = { -1.f, -1.f };
	m_circleVertices[idx + 1].unitPosition = { -1.f,  1.f };
	m_circleVertices[idx + 2].unitPosition = {  1.f,  1.f };
	m_circleVertices[idx + 3].unitPosition = {  1.f, -1.f };
	/** Colour */
	m_circleVertices[idx + 0].colour = colour;
	m_circleVertices[idx + 1].colour = colour;
	m_circleVertices[idx + 2].colour = colour;
	m_circleVertices[idx + 3].colour = colour;
	/** Thickness */
	m_circleVertices[idx + 0].thickness = thickness;
	m_circleVertices[idx + 1].thickness = thickness;
	m_circleVertices[idx + 2].thickness = thickness;
	m_circleVertices[idx + 3].thickness = thickness;

	m_currentCircleCount++;

	if (idx + 3 >= m_circleVertices.size()) spdlog::error("UI Circle vertices have reached limit");
}