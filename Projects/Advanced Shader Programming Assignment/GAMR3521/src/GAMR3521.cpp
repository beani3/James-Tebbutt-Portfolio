#include "GAMR3521.hpp"
#include "camera.hpp"
#include "rotation.hpp"
#include <numeric>
#include <ctime>

MainLayer::MainLayer(GLFWWindowImpl& win) : Layer(win)
{
	m_scene.reset(new Scene);
	m_screenScene.reset(new Scene);
	// Empty SSBO
	std::shared_ptr<SSBO> particleSSBO = std::make_shared<SSBO>(sizeof(Particle) * m_numParts, m_numParts);

	/****************************************
	 * Model Generation
	 ****************************************/

	// Model Shader
	ShaderDescription GPassShaderDesc;
	GPassShaderDesc.type = ShaderType::rasterization;
	GPassShaderDesc.vertexSrcPath = "./assets/shaders/GPassVert.glsl";
	GPassShaderDesc.fragmentSrcPath = "./assets/shaders/GPassFrag.glsl";

	std::shared_ptr<Shader> GPassShader;
	GPassShader = std::make_shared<Shader>(GPassShaderDesc);

	// Pre-Pass Shader
	ShaderDescription prePassShaderDesc;
	prePassShaderDesc.type = ShaderType::rasterization;
	prePassShaderDesc.vertexSrcPath = "./assets/shaders/z-prePassPhongVert.glsl";
	prePassShaderDesc.fragmentSrcPath = "./assets/shaders/z-prePassPhongFrag.glsl";

	std::shared_ptr<Shader> prePassShader;
	prePassShader = std::make_shared<Shader>(prePassShaderDesc);

	//  Model Generation
	VBOLayout modelLayout = {
		{GL_FLOAT, 3}, // Position
		{GL_FLOAT, 3}, // Normal
		{GL_FLOAT, 2}, // UV co-ords
		{GL_FLOAT, 3}  // Tangent
	};

	// Pre-Pass Layout
	VBOLayout depthLayout = { {GL_FLOAT, 3} };

	uint32_t attributeTypes = Model::VertexFlags::positions |
		Model::VertexFlags::normals |
		Model::VertexFlags::uvs |
		Model::VertexFlags::tangents;

	Model shipModel("./assets/models/highPolyShip/highPolyShip.obj", attributeTypes);

	m_modelAlbedo = std::make_shared<Texture>("./assets/models/highPolyShip/textures/albedo.png");
	m_modelMetallic = std::make_shared<Texture>("./assets/models/highPolyShip/textures/metallic.png");
	m_modelNormal = std::make_shared<Texture>("./assets/models/highPolyShip/textures/normal.png");
	m_modelRoughness = std::make_shared<Texture>("./assets/models/highPolyShip/textures/roughness.png");

	// Model Material
	std::shared_ptr<Material> modelMaterial;
	modelMaterial = std::make_shared<Material>(GPassShader, "u_model");
	modelMaterial->setValue("u_albedoMap", m_modelAlbedo);
	modelMaterial->setValue("u_normalMap", m_modelNormal);
	modelMaterial->setValue("u_metallicMap", m_modelMetallic);
	modelMaterial->setValue("u_roughnessMap", m_modelRoughness);

	// Pre-Pass Material
	std::shared_ptr<Material> prePassMaterial;
	prePassMaterial = std::make_shared<Material>(prePassShader, "u_model");

	// Model VAO
	std::shared_ptr<VAO> modelVAO;
	modelVAO = std::make_shared<VAO>(shipModel.m_meshes[0].indices);
	modelVAO->addVertexBuffer(shipModel.m_meshes[0].vertices, modelLayout);

	// Pre-Pass VAO
	std::shared_ptr<VAO> prePassVAO;
	prePassVAO = std::make_shared<VAO>(shipModel.m_meshes[0].indices);
	prePassVAO->addVertexBuffer(shipModel.m_meshes[0].positions, depthLayout);

	m_model.geometry = modelVAO;
	m_model.material = modelMaterial;
	m_model.depthGeometry = prePassVAO;
	m_model.depthMaterial = prePassMaterial;

	m_model.translation = glm::vec3(10.f, 25.f, 10.f);
	m_model.scale = glm::vec3(.5f);
	m_model.recalc();

	m_modelIdx = m_scene->m_actors.size();
	m_scene->m_actors.push_back(m_model);

	/**************************************** 
	* Floor 
	****************************************/

	// Floor Shader
	ShaderDescription floorGPassShaderDesc;
	floorGPassShaderDesc.type = ShaderType::tessellationAndGeometry;
	floorGPassShaderDesc.vertexSrcPath = "./assets/shaders/floorGPassVert.glsl";
	floorGPassShaderDesc.controlSrcPath = "./assets/shaders/floorGPassTCS.glsl";
	floorGPassShaderDesc.evaluationSrcPath = "./assets/shaders/floorGPassTES.glsl";
	floorGPassShaderDesc.geometrySrcPath = "./assets/shaders/floorGPassGeo.glsl";
	floorGPassShaderDesc.fragmentSrcPath = "./assets/shaders/floorGPassFrag.glsl";

	std::shared_ptr<Shader> floorGPassShader;
	floorGPassShader = std::make_shared<Shader>(floorGPassShaderDesc);

	// Floor Generation
	Model floorModel("./assets/models/whitecube/whitecube.obj");
	m_floorAlbedo = std::make_shared<Texture>("./assets/textures/floorPBRTextures/albedo.jpg");
	m_floorNormal = std::make_shared<Texture>("./assets/textures/floorPBRTextures/normal.jpg");
	m_floorMetallic = std::make_shared<Texture>("./assets/textures/floorPBRTextures/AOMetallic.jpg");
	m_floorRoughness = std::make_shared<Texture>("./assets/textures/floorPBRTextures/roughness.jpg");

	VBOLayout floorLayout = {
		{GL_FLOAT, 3}, // Position
		{GL_FLOAT, 2}  // UV co-ords         
	};
	 
	//  Floor Material
	m_floorMaterial = std::make_shared<Material>(floorGPassShader, "u_model");
	m_floorMaterial->setValue("u_albedoMap", m_floorAlbedo);
	m_floorMaterial->setValue("u_normalMap", m_floorNormal);
	m_floorMaterial->setValue("u_metallicMap", m_floorMetallic);
	m_floorMaterial->setValue("u_roughnessMap", m_floorRoughness);
	m_floorMaterial->setValue("u_lightSpaceTransform", shadowPass.camera.projection * shadowView);
	m_floorMaterial->setValue("u_sf", m_sf);
	m_floorMaterial->setPrimitive(GL_PATCHES);
	// Toggle uniforms
	m_floorMaterial->setValue("u_textured", 0);
	m_floorMaterial->setValue("u_normalToggle", 0);

	std::shared_ptr<Grid> grid = std::make_shared<Grid>();
	std::vector<float> floorVertices = grid->getVertices();
	std::vector<unsigned int> floorIndices = grid->getIndices();  

	std::shared_ptr<VAO> floorVAO;
	floorVAO = std::make_shared<VAO>(floorIndices); 
	floorVAO->addVertexBuffer(floorVertices, floorLayout);

	std::vector<float> floor_positions = grid->getVertexPositions();

	std::shared_ptr<VAO> floorPrePassVAO;
	floorPrePassVAO = std::make_shared<VAO>(floorIndices);
	floorPrePassVAO->addVertexBuffer(floor_positions, depthLayout);

	Actor floor;
	floor.geometry = floorVAO;
	floor.material = m_floorMaterial;
	floor.depthGeometry = floorPrePassVAO;
	// Uses the same pre-pass material as the model
	// floor.depthMaterial = prePassMaterial;

	floor.translation = glm::vec3(-80.f, -5.f, -80.f);
	floor.recalc();

	m_floorIdx = m_scene->m_actors.size();
	m_scene->m_actors.push_back(floor);

	/****************************************
	* Lighting and Camera 
	****************************************/

	DirectionalLight dl;
	dl.direction = glm::normalize(m_dlDirection);
	m_scene->m_directionalLights.push_back(dl);

	PointLight pl;
	for (int i = 0; i < numPointLights; i++) {
		pl.colour = glm::vec3(Randomiser::uniformFloatBetween(0.0, 1.0), Randomiser::uniformFloatBetween(0.0, 1.0), Randomiser::uniformFloatBetween(0.0, 1.0));
		pl.position = glm::vec3(Randomiser::uniformFloatBetween(-100.0, 100.0), 5.f, Randomiser::uniformFloatBetween(-100.0, 100.0));
		pl.constants = m_attn;
		m_scene->m_pointLights.push_back(pl);
	}

	FBOLayout typicalLayout = {
		{AttachmentType::ColourHDR, true, true},
		{AttachmentType::Depth, false, false}
	};

	FBOLayout colAndDepthLayout = {
		{AttachmentType::ColourHDR, true, true},
		{AttachmentType::Depth, true, false}
	};

	FBOLayout depthPrepassLayout = {
		{AttachmentType::Depth, true, false}
	};

	FBOLayout GBuffer = {
		{AttachmentType::ColourHDR, true},
		{AttachmentType::ColourHDR, true},
		{AttachmentType::ColourHDR, true},
		{AttachmentType::ColourHDR, true},
		{AttachmentType::Depth, true}
	};

	Actor camera;
	m_cameraIdx = m_scene->m_actors.size();
	camera.translation = glm::vec3(-50.0f, 30.0f, -50.0f);
	camera.rotation = glm::vec3(0.f, 180.f, 0.f);
	camera.recalc();
	m_scene->m_actors.push_back(camera);

	/****************************************
	* Shadow Pre-Pass
	****************************************/

	float orthoSize = m_shadowMapVars.orthoSize; 
	glm::vec3 lightPosition = (m_shadowMapVars.centre - dl.direction) * m_shadowMapVars.distanceAlongLightVector;
	shadowView = glm::lookAt(lightPosition, m_shadowMapVars.centre, m_shadowMapVars.UP);

	shadowPass.scene = m_scene;        
	shadowPass.parseScene();
	shadowPass.target = std::make_shared<FBO>(glm::ivec2(4096, 4096), depthPrepassLayout);

	shadowPass.viewPort = { 0, 0, 4096, 4096 };
	shadowPass.camera.view = shadowView;

	// glm::ortho(left, right, bottom, top, nearPlane, farPlane)
	shadowPass.camera.projection = glm::ortho(-orthoSize, orthoSize, -orthoSize, orthoSize, 0.1f, 200.f);
	shadowPass.setCachedValue("b_lightCamera", "u_view", shadowPass.camera.view);
	shadowPass.setCachedValue("b_lightCamera", "u_projection", shadowPass.camera.projection);

	m_shadowPPIdx = m_renderer.getPassCount();
	m_renderer.addDepthPass(shadowPass);

	/****************************************
	* Compute Pass
	****************************************/
	
	TextureDescription texDesc;
	texDesc.width = 512;
	texDesc.height = 512;
	texDesc.channels = 4;
	texDesc.type = TextureDataType::HDR;
	texDesc.wrapMethod = GL_CLAMP_TO_EDGE;

	std::shared_ptr<Texture> emptyTexture1;
	emptyTexture1 = std::make_shared<Texture>(texDesc);

	ShaderDescription textureComputeDesc;
	textureComputeDesc.type = ShaderType::compute;
	textureComputeDesc.computeSrcPath = "./assets/shaders/computeTexture.glsl";

	std::shared_ptr<Shader> computeShader;
	computeShader = std::make_shared<Shader>(textureComputeDesc);

	std::shared_ptr<Material> computeMaterial;
	computeMaterial = std::make_shared<Material>(computeShader);
	computeMaterial->setValue("u_frequency", m_frequency);
	computeMaterial->setValue("u_amplitude", m_amplitude);
	computeMaterial->setValue("u_octaves", m_octaves);
	computeMaterial->setValue("u_lacunarity", m_lacunarity);
	computeMaterial->setValue("u_persistance", m_persistance);

	ComputePass textureComputePass;
	textureComputePass.material = computeMaterial;
	textureComputePass.workgroups = { 32, 32, 1 };
	textureComputePass.barrier = MemoryBarrier::ShaderImageAccess;

	ImageDescWithTexture noiseDesc;
	noiseDesc.texture = emptyTexture1;
	noiseDesc.imageUnit = textureComputePass.material->m_shader->m_imageBindingPoints["noiseImg"];
	noiseDesc.access = TextureAccess::WriteOnly;

	textureComputePass.images.push_back(Image(noiseDesc));

	m_computeIdx = m_renderer.getPassCount();
	m_renderer.addComputePass(textureComputePass);

	/****************************************
	* CDM Pass
	****************************************/

	std::shared_ptr<Texture> emptyTexture2;
	emptyTexture2 = std::make_shared<Texture>(texDesc);

	ShaderDescription textureCDMDesc;
	textureCDMDesc.type = ShaderType::compute;
	textureCDMDesc.computeSrcPath = "./assets/shaders/CDMTexture.glsl";

	std::shared_ptr<Shader> cdmShader;
	cdmShader = std::make_shared<Shader>(textureCDMDesc);

	std::shared_ptr<Material> cdmMaterial;
	cdmMaterial = std::make_shared<Material>(cdmShader);
	cdmMaterial->setValue("u_noiseTexture", m_renderer.getComputePass(m_computeIdx).images[0].getTexture());

	ComputePass CDMPass;
	CDMPass.material = cdmMaterial;
	CDMPass.workgroups = { 32, 32, 1 };
	CDMPass.barrier = MemoryBarrier::ShaderImageAccess;

	ImageDescWithTexture cdmDesc;
	cdmDesc.texture = emptyTexture2;
	cdmDesc.imageUnit = CDMPass.material->m_shader->m_imageBindingPoints["cdmImg"];
	cdmDesc.access = TextureAccess::WriteOnly;

	CDMPass.images.push_back(Image(cdmDesc));

	m_cdmIdx = m_renderer.getPassCount();
	m_renderer.addComputePass(CDMPass);

	/****************************************
	* Particle Setup Pass
	****************************************/

	ShaderDescription initPartShaderDesc;
	initPartShaderDesc.type = ShaderType::compute;
	initPartShaderDesc.computeSrcPath = "./assets/shaders/initParticle.glsl";

	std::shared_ptr<Shader> initPartShader;
	initPartShader = std::make_shared<Shader>(initPartShaderDesc);

	std::shared_ptr<Material> initPartMaterial;
	initPartMaterial = std::make_shared<Material>(initPartShader);
	initPartMaterial->setValue("u_origin", glm::vec3(0.f, 10.f, 0.f));
	float maxAge = 5.f;
	initPartMaterial->setValue("u_age", Randomiser::uniformFloatBetween(1.f, maxAge));

	ComputePass initPartPass;
	initPartPass.ssbo = particleSSBO;
	initPartPass.material = initPartMaterial;
	initPartPass.workgroups = {32, 32, 1};
	initPartPass.barrier = MemoryBarrier::ShaderImageAccess;

	auto ssboParticles = particleSSBO->writeToCPU<Particle>();

	m_initPartIdx = m_renderer.getPassCount();
	m_renderer.addComputePass(initPartPass);

	/****************************************
	* GPass
	****************************************/

	RenderPass GPass;
	GPass.scene = m_scene;
	GPass.parseScene();
	// GPass.target = std::make_shared<FBO>(); // Default framebuffer
	// GPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout); // Scene for post-processing 
	// GPass.target = std::make_shared<FBO>(m_winRef.getSize(), colAndDepthLayout); // Scene for depth attachment
	GPass.target = std::make_shared<FBO>(m_winRef.getSize(), GBuffer); // Scene for deferred rendering
	GPass.camera.projection = glm::perspective(45.f, m_winRef.getWidthf() / m_winRef.getHeightf(), 0.1f, 1000.f);
	GPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	GPass.camera.updateView(m_scene->m_actors.at(m_cameraIdx).transform);
	m_floorMaterial->setValue("u_heightMap", m_renderer.getComputePass(m_cdmIdx).images[0].getTexture());
	//m_floorMaterial->setValue("u_heightMap", m_floorHeight);

	m_scene->m_actors.at(m_cameraIdx).attachScript<CameraScript>(GPass.scene->m_actors.at(m_cameraIdx), m_winRef, glm::vec3(2.5f, 1.5f, 2.5f), 2.f);
	m_scene->m_actors.at(m_modelIdx).attachScript<RotationScript>(m_scene->m_actors.at(m_modelIdx), glm::vec3(0.0, -1.0, 0.0), GLFW_KEY_P);

	GPass.setCachedValue("b_camera", "u_view", GPass.camera.view);
	GPass.setCachedValue("b_camera", "u_projection", GPass.camera.projection);

	// GPass.prePassActions.push_back([]() {glDisable(GL_BLEND); });
	// GPass.postPassActions.push_back([]() {glEnable(GL_BLEND); });
	m_gpassIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(GPass);

	/*******************************************
	* Quad for Light Pass and Post-Processing
	********************************************/

	// Quad Generation
	float width = m_winRef.getWidthf();
	float height = m_winRef.getHeightf();

	const std::vector<float> screenVertices = {
		//  Positions		      UVs			  	
			0.0f,	0.0f,	0.0f, 0.0f, 1.0f,	// Bottom-left corner
			width,	0.0f,	0.0f, 1.0f, 1.0f,	// Bottom-rigt corner
			width,	height, 0.0f, 1.0f, 0.0f,	// Top-right corner
			0.0f,	height, 0.0f, 0.0f, 0.0f	// Top-left corner 
	};
	const std::vector<uint32_t> screenIndices = { 0, 1, 2, 2, 3, 0 };

	VBOLayout screenQuadLayout = {
		{GL_FLOAT, 3},
		{GL_FLOAT, 2}
	};

	std::shared_ptr<VAO> quadVAO;
	quadVAO = std::make_shared<VAO>(screenIndices);
	quadVAO->addVertexBuffer(screenVertices, screenQuadLayout);

	/****************************************
	* Skybox Pass
	****************************************/

	// Skybox Shader
	ShaderDescription skyboxShaderDesc;
	skyboxShaderDesc.type = ShaderType::rasterization;
	skyboxShaderDesc.vertexSrcPath = "./assets/shaders/skyboxVert.glsl";
	skyboxShaderDesc.fragmentSrcPath = "./assets/shaders/skyboxFrag.glsl";

	std::shared_ptr<Shader> skyboxPhongShader;
	skyboxPhongShader = std::make_shared<Shader>(skyboxShaderDesc);

	// Skybox Generation
	std::array<const char*, 6> cubeMapPaths = {
		"./assets/textures/spaceSkybox/right.png",
		"./assets/textures/spaceSkybox/left.png",
		"./assets/textures/spaceSkybox/top.png",
		"./assets/textures/spaceSkybox/bottom.png",
		"./assets/textures/spaceSkybox/front.png",
		"./assets/textures/spaceSkybox/back.png"
	};
	std::shared_ptr<CubeMap> cubeMap = std::make_shared<CubeMap>(cubeMapPaths, false, false);

	// Skybox Material
	std::shared_ptr<Material> skyboxMaterial;
	skyboxMaterial = std::make_shared<Material>(skyboxPhongShader, "u_model");
	skyboxMaterial->setValue("u_skybox", cubeMap);

	std::vector<uint32_t> skyboxIndices(skyboxVertices.size() / 3);
	std::iota(skyboxIndices.begin(), skyboxIndices.end(), 0);

	std::shared_ptr<VAO> skyboxVAO;
	skyboxVAO = std::make_shared<VAO>(skyboxIndices);
	skyboxVAO->addVertexBuffer(skyboxVertices, { { GL_FLOAT, 3 } });

	m_skybox.geometry = skyboxVAO;
	m_skybox.material = skyboxMaterial;
	m_sbIdx = m_screenScene->m_actors.size();
	m_screenScene->m_actors.push_back(m_skybox);

	RenderPass skyboxPass;
	skyboxPass.scene = m_screenScene;
	skyboxPass.parseScene();
	skyboxPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout); // Default FBO
	skyboxPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	skyboxPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	skyboxPass.setCachedValue("b_camera2D", "u_view", skyboxPass.camera.view);
	skyboxPass.setCachedValue("b_camera2D", "u_projection", skyboxPass.camera.projection);

	m_skyboxIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(skyboxPass);

	/****************************************
	* Light Pass
	****************************************/
	m_screenScene.reset(new Scene);

	// Light Pass Shader
	ShaderDescription lightPassShaderDesc;
	lightPassShaderDesc.type = ShaderType::rasterization;
	lightPassShaderDesc.vertexSrcPath = "./assets/shaders/lightPassVert.glsl";
	lightPassShaderDesc.fragmentSrcPath = "./assets/shaders/lightPassFrag.glsl";
	lightPassShaderDesc.fragmentSrcPath = "./assets/shaders/lightPassFrag.glsl";

	std::shared_ptr<Shader> lightPassShader;
	lightPassShader = std::make_shared<Shader>(lightPassShaderDesc);

	// Light Pass Material
	m_lightPassMaterial = std::make_shared<Material>(lightPassShader, "u_model");
	m_lightPassMaterial->setValue("u_gBuffPosInWS", GPass.target->getTarget(0));
	m_lightPassMaterial->setValue("u_gBuffNormal", GPass.target->getTarget(1));
	m_lightPassMaterial->setValue("u_gBuffDiffSpec", GPass.target->getTarget(2));
	m_lightPassMaterial->setValue("u_gBuffRoughMetAO", GPass.target->getTarget(3));
	m_lightPassMaterial->setValue("u_depthAttachment", GPass.target->getTarget(4));
	m_lightPassMaterial->setValue("u_skyboxCol", skyboxPass.target->getTarget(0));
	m_lightPassMaterial->setValue("u_viewPos", GPass.scene->m_actors.at(m_cameraIdx).translation);
	m_lightPassMaterial->setValue("u_ambientFactor", m_ambientFactor);
	m_lightPassMaterial->setValue("u_metallic", m_metallic);
	m_lightPassMaterial->setValue("u_roughness", m_roughness);
	m_model.material = m_lightPassMaterial;
	floor.material = m_lightPassMaterial; 

	Actor lightQuad;
	lightQuad.geometry = quadVAO;
	lightQuad.material = m_lightPassMaterial;
	m_screenScene->m_actors.push_back(lightQuad);

	RenderPass lightPass;
	lightPass.scene = m_screenScene;
	lightPass.parseScene();
	lightPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout); 
	lightPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	lightPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	lightPass.setCachedValue("b_camera2D", "u_view", lightPass.camera.view);
	lightPass.setCachedValue("b_camera2D", "u_projection", lightPass.camera.projection);

	lightPass.setCachedValue("b_lights", "u_viewPos", m_scene->m_actors.at(m_cameraIdx).translation);
	lightPass.setCachedValue("b_lights", "dLight.colour", m_scene->m_directionalLights.at(0).colour);
	lightPass.setCachedValue("b_lights", "dLight.direction", m_scene->m_directionalLights.at(0).direction);
	// skyboxMaterial->setValue("u_skyboxView", lightPass.camera.view);

	for (int i = 0; i < numPointLights; i++) {
		lightPass.setCachedValue("b_lights", "pLights[" + std::to_string(i) + "].colour", m_scene->m_pointLights.at(i).colour);
		lightPass.setCachedValue("b_lights", "pLights[" + std::to_string(i) + "].position", m_scene->m_pointLights.at(i).position);
		lightPass.setCachedValue("b_lights", "pLights[" + std::to_string(i) + "].constants", m_scene->m_pointLights.at(i).constants);
	}

	m_lightIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(lightPass);

	/****************************************
	* Blur Pass
	****************************************/
	m_screenScene.reset(new Scene);

	// Blur Shader
	ShaderDescription blurShaderDesc;
	blurShaderDesc.type = ShaderType::rasterization;
	blurShaderDesc.vertexSrcPath = "./assets/shaders/blurPhongVert.glsl";
	blurShaderDesc.fragmentSrcPath = "./assets/shaders/blurPhongFrag.glsl";

	std::shared_ptr<Shader> blurShader;
	blurShader = std::make_shared<Shader>(blurShaderDesc);

	// Setting material for quad
	m_postProcessingMaterial = std::make_shared<Material>(blurShader, "u_model");
	m_postProcessingMaterial->setValue("u_inputTexture", lightPass.target->getTarget(0));
	m_imageSize = glm::vec2(GPass.target->getTarget(0)->getWidth(), GPass.target->getTarget(0)->getHeight());
	m_postProcessingMaterial->setValue("u_imageSize", m_imageSize);
	m_postProcessingMaterial->setValue("u_radius", m_radius);
	
	Actor blurQuad;
	blurQuad.geometry = quadVAO;
	blurQuad.material = m_postProcessingMaterial;
	m_screenScene->m_actors.push_back(blurQuad);

	RenderPass blurPass;
	blurPass.scene = m_screenScene;
	blurPass.parseScene();
	blurPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout); // Default FBO
	blurPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	blurPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	blurPass.setCachedValue("b_camera2D", "u_view", blurPass.camera.view);
	blurPass.setCachedValue("b_camera2D", "u_projection", blurPass.camera.projection);

	m_blurIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(blurPass);

	/****************************************
	* Depth Visualisation Pass
	****************************************/
	m_screenScene.reset(new Scene);

	// Depth Shader
	ShaderDescription depthShaderDesc;
	depthShaderDesc.type = ShaderType::rasterization;
	depthShaderDesc.vertexSrcPath = "./assets/shaders/depthPhongVert.glsl";
	depthShaderDesc.fragmentSrcPath = "./assets/shaders/depthPhongFrag.glsl";

	std::shared_ptr<Shader> depthPhongShader;
	depthPhongShader = std::make_shared<Shader>(depthShaderDesc);

	std::shared_ptr<Material> depthMaterial;
	depthMaterial = std::make_shared<Material>(depthPhongShader, "u_model");
	depthMaterial->setValue("u_inputTexture", GPass.target->getTarget(4));
	depthMaterial->setValue("u_near", m_near);
	depthMaterial->setValue("u_far", m_far);

	Actor depthQuad;
	depthQuad.geometry = quadVAO;
	depthQuad.material = depthMaterial;
	m_screenScene->m_actors.push_back(depthQuad);

	RenderPass depthPass;
	depthPass.scene = m_screenScene;
	depthPass.parseScene();
	depthPass.target = std::make_shared<FBO>(m_winRef.getSize(), colAndDepthLayout);
	depthPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	depthPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	depthPass.setCachedValue("b_camera2D", "u_view", depthPass.camera.view);
	depthPass.setCachedValue("b_camera2D", "u_projection", depthPass.camera.projection);

	m_depthIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(depthPass);

	/****************************************
	* Fog Pass
	****************************************/
	m_screenScene.reset(new Scene);

	// Fog Shader
	ShaderDescription fogShaderDesc;
	fogShaderDesc.type = ShaderType::rasterization;
	fogShaderDesc.vertexSrcPath = "./assets/shaders/fogPhongVert.glsl";
	fogShaderDesc.fragmentSrcPath = "./assets/shaders/fogPhongFrag.glsl";

	std::shared_ptr<Shader> fogPhongShader;
	fogPhongShader = std::make_shared<Shader>(fogShaderDesc);

	std::shared_ptr<Material> fogMaterial;
	fogMaterial = std::make_shared<Material>(fogPhongShader, "u_model");
	fogMaterial->setValue("u_inputTexture", lightPass.target->getTarget(0));
	fogMaterial->setValue("u_depthTexture", GPass.target->getTarget(4));

	Actor fogQuad;
	fogQuad.geometry = quadVAO;
	fogQuad.material = fogMaterial;
	m_screenScene->m_actors.push_back(fogQuad);

	RenderPass fogPass;
	fogPass.scene = m_screenScene;
	fogPass.parseScene();
	fogPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout);
	fogPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	fogPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	fogPass.setCachedValue("b_camera2D", "u_view", fogPass.camera.view);
	fogPass.setCachedValue("b_camera2D", "u_projection", fogPass.camera.projection);

	m_fogIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(fogPass);

	/****************************************
	* Depth of Field Pass
	****************************************/
	m_screenScene.reset(new Scene);

	ShaderDescription dofShaderDesc;
	dofShaderDesc.type = ShaderType::rasterization;
	dofShaderDesc.vertexSrcPath = "./assets/shaders/dofPhongVert.glsl";
	dofShaderDesc.fragmentSrcPath = "./assets/shaders/dofPhongFrag.glsl";

	std::shared_ptr<Shader> dofPhongShader;
	dofPhongShader = std::make_shared<Shader>(dofShaderDesc);

	std::shared_ptr<Material> dofMaterial;
	dofMaterial = std::make_shared<Material>(dofPhongShader, "u_model");
	dofMaterial->setValue("u_inputTexture", lightPass.target->getTarget(0));
	dofMaterial->setValue("u_blurTexture", blurPass.target->getTarget(0));
	dofMaterial->setValue("u_depthTexture", depthPass.target->getTarget(0));

	Actor dofQuad;
	dofQuad.material = dofMaterial;
	dofQuad.geometry = quadVAO;
	m_screenScene->m_actors.push_back(dofQuad);

	RenderPass dofPass;
	dofPass.scene = m_screenScene;
	dofPass.parseScene();
	dofPass.target = std::make_shared<FBO>(m_winRef.getSize(), typicalLayout);
	dofPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	dofPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	dofPass.setCachedValue("b_camera2D", "u_view", dofPass.camera.view);
	dofPass.setCachedValue("b_camera2D", "u_projection", dofPass.camera.projection);

	m_dofIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(dofPass);

	/****************************************
	* Shadow Visualisation Pass
	****************************************/
	m_screenScene.reset(new Scene);
	   
	// Shadow Visualisation Shader
	ShaderDescription shadowShaderDesc;
	shadowShaderDesc.type = ShaderType::rasterization;
	shadowShaderDesc.vertexSrcPath = "./assets/shaders/shadowPhongVert.glsl"; 
	shadowShaderDesc.fragmentSrcPath = "./assets/shaders/shadowPhongFrag.glsl";

	std::shared_ptr<Shader> shadowPhongShader;
	shadowPhongShader = std::make_shared<Shader>(shadowShaderDesc);
	 
	std::shared_ptr<Material> shadowMaterial;
	shadowMaterial = std::make_shared<Material>(shadowPhongShader, "u_model");
	shadowMaterial->setValue("u_inputTexture", GPass.target->getTarget(2));

	m_lightPassMaterial->setValue("u_lightSpaceTransform", shadowPass.camera.projection * shadowView);
	m_lightPassMaterial->setValue("u_shadowMap", shadowPass.target->getTarget(0));  

	Actor shadowQuad;
	shadowQuad.material = shadowMaterial;
	shadowQuad.geometry = quadVAO; 
	m_screenScene->m_actors.push_back(shadowQuad);

	RenderPass shadowVisPass;
	shadowVisPass.scene = m_screenScene;
	shadowVisPass.parseScene();
	shadowVisPass.target = std::make_shared<FBO>(m_winRef.getSize(), colAndDepthLayout);
	shadowVisPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	shadowVisPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	shadowVisPass.setCachedValue("b_camera2D", "u_view", shadowVisPass.camera.view);
	shadowVisPass.setCachedValue("b_camera2D", "u_projection", shadowVisPass.camera.projection);

	m_shadowIdx = m_renderer.getPassCount();
	m_renderer.addRenderPass(shadowVisPass);

	/****************************************
	* Tone Mapping
	****************************************/
	m_screenScene.reset(new Scene);

	// Tone Mapping Shader
	ShaderDescription toneMapShaderDesc;
	toneMapShaderDesc.type = ShaderType::rasterization;
	toneMapShaderDesc.vertexSrcPath = "./assets/shaders/toneMappingPhongVert.glsl";
	toneMapShaderDesc.fragmentSrcPath = "./assets/shaders/toneMappingPhongFrag.glsl";

	std::shared_ptr<Shader> toneMapPhongShader;
	toneMapPhongShader = std::make_shared<Shader>(toneMapShaderDesc);

	std::shared_ptr<Material> toneMapMaterial;
	toneMapMaterial = std::make_shared<Material>(toneMapPhongShader, "u_model");
	toneMapMaterial->setValue("u_inputTexture", dofPass.target->getTarget(0));

	Actor toneMapQuad;
	toneMapQuad.material = toneMapMaterial;
	toneMapQuad.geometry = quadVAO;
	m_screenScene->m_actors.push_back(toneMapQuad);

	RenderPass toneMapPass;
	toneMapPass.scene = m_screenScene;
	toneMapPass.parseScene();
	toneMapPass.target = std::make_shared<FBO>(); // Default FBO
	toneMapPass.viewPort = { 0, 0, m_winRef.getWidth(), m_winRef.getHeight() };

	toneMapPass.camera.projection = glm::ortho(0.f, width, height, 0.f);
	toneMapPass.setCachedValue("b_camera2D", "u_view", toneMapPass.camera.view);
	toneMapPass.setCachedValue("b_camera2D", "u_projection", toneMapPass.camera.projection);

	m_renderer.addRenderPass(toneMapPass);
}

void MainLayer::onRender() const
{
	m_renderer.render();
}

void MainLayer::onUpdate(float timestep)
{
	glDisable(GL_BLEND);

	// Update scripts
	for (auto& actor : m_scene->m_actors)
	{
		actor.onUpdate(timestep);
	}

	// Update camera  and its position in UBO
	auto& camera = m_scene->m_actors.at(m_cameraIdx);
	 
	auto& gPass = m_renderer.getRenderPass(m_gpassIdx);
	auto& lPass = m_renderer.getRenderPass(m_lightIdx);

	gPass.camera.updateView(camera.transform);
	gPass.setCachedValue("b_camera", "u_view", gPass.camera.view);
	gPass.setCachedValue("b_camera", "u_viewPos", camera.translation);
	m_skybox.material->setValue("u_skyboxView", glm::mat4(glm::mat3(gPass.camera.view)));

	lPass.scene->m_actors[0].material->setValue("u_viewPos", camera.translation);

	glm::vec3 lightPosition = (m_shadowMapVars.centre - glm::normalize(m_dlDirection)) * m_shadowMapVars.distanceAlongLightVector;
	glm::mat4 shadowView = glm::lookAt(lightPosition, m_shadowMapVars.centre, m_shadowMapVars.UP);
}

void MainLayer::onImGUIRender()
{
	ImGui::Begin("Scene Changes");
	ImVec2 imageSize = ImVec2(256, 256);
	ImVec2 uv0 = ImVec2(0.0f, 1.0f);
	ImVec2 uv1 = ImVec2(1.0f, 0.0f);

	GLuint textureID00 = m_renderer.getRenderPass(m_gpassIdx).target->getTarget(0)->getID();	// GBuffer 0
	ImGui::Image((void*)(intptr_t)textureID00, imageSize, uv0, uv1);
	ImGui::SameLine();

	textureID00 = m_renderer.getRenderPass(m_gpassIdx).target->getTarget(1)->getID();	// GBuffer 1
	ImGui::Image((void*)(intptr_t)textureID00, imageSize, uv0, uv1);
	textureID00 = m_renderer.getRenderPass(m_gpassIdx).target->getTarget(2)->getID(); // GBuffer 2
	ImGui::Image((void*)(intptr_t)textureID00, imageSize, uv0, uv1);
	ImGui::SameLine();

	if (m_normalToggle == 1) {
		textureID00 = m_renderer.getComputePass(m_cdmIdx).images[0].getTexture()->getID();
		ImGui::Image((void*)(intptr_t)textureID00, imageSize, uv0, uv1);
	}

	ImGui::NewLine();
	if (ImGui::Checkbox("Wireframe ", &m_wireFrame)) {
		auto& GPass = m_renderer.getRenderPass(m_gpassIdx);
		if (m_wireFrame) {
			GPass.prePassActions.clear();
			GPass.prePassActions.emplace_back([]() {glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);});
		}
		else {
			GPass.prePassActions.clear();
			GPass.prePassActions.emplace_back([]() {glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);});
		}
	}
	ImGui::SameLine();
	if (ImGui::Checkbox("Textured", &m_textured)) {
		if (m_textured) {
			m_floorMaterial->setValue("u_textured", 1);
		}
		else {
			m_floorMaterial->setValue("u_textured", 0);
		}
	}
	ImGui::SameLine();
	ImGui::RadioButton("SN", &m_normalToggle, 0); ImGui::SameLine();
	ImGui::RadioButton("CDM", &m_normalToggle, 1); ImGui::SameLine();
	ImGui::RadioButton("PF", &m_normalToggle, 2);
	switch (m_normalToggle) {
	case 0:
		m_floorMaterial->setValue("u_normalToggle", 0);
		break;
	case 1:
		m_floorMaterial->setValue("u_normalToggle", 1);
		break;
	case 2:
		m_floorMaterial->setValue("u_normalToggle", 2);
		break;
	default:
		break;
	}

	auto& blurPass = m_renderer.getRenderPass(m_blurIdx);
	if (ImGui::SliderInt("Blur Radius", &m_radius, 0, 25)) {
		blurPass.scene->m_actors[0].material->setValue("u_radius", m_radius);
	}
	auto& depthPass = m_renderer.getRenderPass(m_depthIdx);
	if (ImGui::SliderFloat("In focus", &m_far, 10.f, 100.f)) {
		depthPass.scene->m_actors[0].material->setValue("u_far", m_far);
	}
	if (ImGui::SliderFloat("Out of focus", &m_near, 0.1f, 2.f)) {
		depthPass.scene->m_actors[0].material->setValue("u_near", m_near);
	}
	if (ImGui::SliderFloat("Ambient Factor", &m_ambientFactor, 0.f, 1.f)) {
		m_lightPassMaterial->setValue("u_ambientFactor", m_ambientFactor);
	}
	if (ImGui::SliderFloat("Metallic", &m_metallic, 0.f, 0.6f)) {
		m_lightPassMaterial->setValue("u_metallic", m_metallic);
	}
	if (ImGui::SliderFloat("Roughness", &m_roughness, 0.f, 0.6f)) {
		m_lightPassMaterial->setValue("u_roughness", m_roughness);
	}

	ImGui::End();

	ImGui::Begin("Shadow Visualisation");
	GLuint textureID01 = m_renderer.getDepthPass(0).target->getTarget(0)->getID();

	ImGui::Image((void*)(intptr_t)textureID01, imageSize, uv0, uv1);

	float dir[3] = { m_dlDirection.x, m_dlDirection.y, m_dlDirection.z };
	if (ImGui::DragFloat3("Light Direction", dir)) {
		auto& shadowPass = m_renderer.getDepthPass(m_shadowPPIdx);
		m_dlDirection = glm::vec3(dir[0], dir[1], dir[2]);
		glm::clamp(m_dlDirection);
		glm::vec3 lightPosition = (m_shadowMapVars.centre - glm::normalize(m_dlDirection)) * m_shadowMapVars.distanceAlongLightVector;
		glm::mat4 shadowView = glm::lookAt(lightPosition, m_shadowMapVars.centre, m_shadowMapVars.UP);

		shadowPass.camera.view = shadowView;

		shadowPass.setCachedValue("b_lightCamera", "u_view", shadowPass.camera.view);
		shadowPass.setCachedValue("b_lightCamera", "u_projection", shadowPass.camera.projection);
		shadowPass.setCachedValue("b_lightCamera", "u_viewPos", lightPosition);

		m_lightPassMaterial->setValue("u_lightSpaceTransform", shadowPass.camera.projection * shadowView);
	}

	ImGui::End();

	ImGui::Begin("Compute Shader Visualisation");
	auto& computePass = m_renderer.getComputePass(m_computeIdx);

	ImVec2 imageSize_ = ImVec2(384, 384);
	GLuint textureID02 = m_renderer.getComputePass(m_computeIdx).images[0].getTexture()->getID();
	ImGui::Image((void*)(intptr_t)textureID02, imageSize_, uv0, uv1);

	if (ImGui::SliderFloat("Heigt Map Scaler", &m_sf, 0.1f, 15.f)) {
		m_floorMaterial->setValue("u_sf", m_sf);
	}
	if (ImGui::SliderInt("Octaves", &m_octaves, 1.f, 8.f)) {
		computePass.material->setValue("u_octaves", m_octaves);
	}
	if (ImGui::SliderFloat("Frequency", &m_frequency, 1.f, 5.f)) {
		computePass.material->setValue("u_frequency", m_frequency);
	}
	if (ImGui::SliderFloat("Amplitude", &m_amplitude, 1.f, 2.f)) {
		computePass.material->setValue("u_amplitude", m_amplitude);
	}
	if (ImGui::SliderFloat("Persistance", &m_persistance, 0.1f, 1.f)) {
		computePass.material->setValue("u_persistance", m_persistance);
	}
	if (ImGui::SliderFloat("Lacunarity", &m_lacunarity, 1.f, 7.5f)) {
		computePass.material->setValue("u_lacunarity", m_lacunarity);
	}

	ImGui::End();

	// ImGui::ShowDemoWindow();
}
