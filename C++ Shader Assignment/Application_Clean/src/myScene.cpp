#pragma once
#include "MyScene.h"
#include "InputHandler.h"
#include "FPSCamera.h"
#include "Textures/TextureManager.h"
#include <random>
#include <vector>

MyScene::MyScene(GLFWwindow* window, InputHandler* H) : Scene(window, H) {
	m_camera = new FirstPersonCamera();
	m_camera->attachHandler(m_window, m_handler);
	m_myShader = new Shader("..\\shaders\\vertex-shader.glsl", "..\\shaders\\fragment-shader.glsl");
	// DirectionalLight(vec3 col, vec3 dir)
	m_directionalLight = new DirectionalLight(glm::vec3(1.0f), glm::vec3(-1.0f, -1.0f, 0.0f));
	// SpotLight(vec3 col, vec3 pos, vec3 attn, vec3 dir, vec2 radii)
	m_spotLight = new SpotLight(glm::vec3(0.0f, 1.0f, 0.0f),
		glm::vec3(0.0f, 3.0f, 0.0f),
		glm::vec3(1.0f, 0.27f, 0.0028f),
		glm::vec3(0.0f, -1.0f, 0.0f),
		glm::vec2(glm::cos(glm::radians(12.5f)), glm::cos(glm::radians(17.5f))));
	planeDiff = TextureManager::LoadTexture("..\\Resources\\Textures\\diffuseFloor.jpg");
	planeSpec = TextureManager::LoadTexture("..\\Resources\\Textures\\specularFloor.jpg");
	planeNorm = TextureManager::LoadTexture("..\\Resources\\Textures\\normalFloor.jpg");
	m_plane = new Plane(planeDiff, planeSpec, planeNorm, 64);
	cubeDiff = TextureManager::LoadTexture("..\\Resources\\Textures\\diffuseCube.jpg");
	cubeSpec = TextureManager::LoadTexture("..\\Resources\\Textures\\specularCube.jpg");
	cubeNorm = TextureManager::LoadTexture("..\\Resources\\Textures\\normalCube.jpg");
	m_cube = new Cube(cubeDiff, cubeSpec, cubeNorm, 64);
	numPL = 30;

	// Generates random values to decide positions and rotations of cubes and point lights 
	//  and randomise
	std::default_random_engine rd;
	std::uniform_int_distribution<int> X_Dist(-6, 6);
	std::uniform_int_distribution<int> Y_Dist(0, 11);
	std::uniform_int_distribution<int> Z_Dist(-6, 6);
	std::uniform_int_distribution<int> R_Dist(0, 1);
	std::uniform_real_distribution<float> C_Dist(0.0f, 1.0f);
	
	// fill the vector with positions
	for (int i = 0; i < numCubes; i++) {
		int x_dist = X_Dist(rd);
		int y_dist = Y_Dist(rd);
		int z_dist = Z_Dist(rd);
		int r_dist = R_Dist(rd);

		glm::vec4 randPosR = glm::vec4(x_dist, y_dist, z_dist, r_dist);
		randPositions.push_back(randPosR);
	}
}

MyScene::~MyScene() {
	delete m_myShader;
	delete m_cube;
	delete m_plane;
	delete m_directionalLight;
}

void MyScene::render() {
	// set camera uniforms, scene wide
	m_myShader->use();
	m_myShader->setMat4("View", m_camera->getViewMatrix());
	m_myShader->setMat4("Projection", m_camera->getProjectionMatrix());
	m_myShader->setVec3("viewPos", m_camera->getPosition());
	// set light uniforms
	m_directionalLight->setLightUniforms(m_myShader);
	for (int i = 0; i < m_pointLight.size(); i++) {
		m_pointLight[i]->setLightUniforms(m_myShader, i);
	}
	m_spotLight->setLightUniforms(m_myShader);
	// set toggle uniforms 
	m_myShader->setInt("useNM", useNM);
	m_myShader->setInt("useDL", useDL);
	m_myShader->setInt("usePL", usePL);
	m_myShader->setInt("useSL", useSL);

	// draw floor plane 
	glBindVertexArray(m_plane->getVAO());
	m_plane->setPlaneMatVals(m_myShader);
	m_plane->setTransform(m_myShader);
	glDrawElements(GL_TRIANGLES, m_plane->getIndiciesCount(), GL_UNSIGNED_INT, 0);

	// draw wall planes
	m_plane->translate(glm::vec3(0.0, 5.0, -5.0));
	m_plane->rotate(glm::radians(90.0), glm::vec3(1.0, 0.0, 0.0));
	m_plane->setTransform(m_myShader);
	glDrawElements(GL_TRIANGLES, m_plane->getIndiciesCount(), GL_UNSIGNED_INT, 0);
	m_plane->translate(glm::vec3(-9.0, 5.0, 0.0));
	m_plane->rotate(1.5708, glm::vec3(0.0, 0.0, 1.0));
	m_plane->setTransform(m_myShader);
	glDrawElements(GL_TRIANGLES, m_plane->getIndiciesCount(), GL_UNSIGNED_INT, 0);
	m_plane->resetTransform();

	genRandCubes();
	// genRandLights();
};

void MyScene::genRandCubes() {
	// loads 50 cubes at random positions with random rotation axis'
	glBindVertexArray(m_cube->getVAO());
	for (int i = 0; i < numCubes; i++) {
		
		glm::vec4 posR = randPositions.at(i);
		glm::vec3 pos = glm::vec3(posR.x, posR.y, posR.z);
		int r = posR.w;

		m_cube->translate(pos);
		m_cube->rotate((float)(glfwGetTime() * 0.5), glm::vec3(r));
		m_cube->setCubeMatVals(m_myShader);
		m_cube->setTransform(m_myShader);
		glDrawElements(GL_TRIANGLES, m_cube->getIndiciesCount(), GL_UNSIGNED_INT, 0);
		m_cube->resetTransform();
	}
}

void MyScene::genRandLights() {
	std::default_random_engine rd;
	std::uniform_int_distribution<int> xz_dist(-6, 6);
	std::uniform_int_distribution<int> y_dist(0, 11);
	std::uniform_real_distribution<float> col_dist(0.0f, 1.0f);

	// loads 30 pointlights at random positions and colours.
	for (int i = 0; i < numPL; i++) {
		// PointLight(vec3 col, vec3 pos, vec3 attn)
		m_pointLight.push_back(new PointLight(glm::vec3(col_dist(rd), col_dist(rd), col_dist(rd)), glm::vec3(xz_dist(rd), y_dist(rd), xz_dist(rd)), glm::vec3(1.0f, 0.0014f, 0.000007f)));
	}
} 

void MyScene::toggleOptions() {
	if (m_handler->isKeyPressed(GLFW_KEY_G)) {
		useDL = !useDL;
	}

	if (m_handler->isKeyPressed(GLFW_KEY_H)) {
		useNM = !useNM;
	}

	if (m_handler->isKeyPressed(GLFW_KEY_J)) {
		usePL = !usePL;
	}

	if (m_handler->isKeyPressed(GLFW_KEY_K)) {
		useSL = !useSL;
	}
}

void MyScene::update(float dt) {
	m_camera->update(dt);
	toggleOptions();
	render();
}