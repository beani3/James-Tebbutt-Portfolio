#pragma once
#include "Scene.h"
#include "Shader.h"
#include "Lights/DirectionalLight.h"
#include "Lights/PointLight.h"
#include "Lights/SpotLight.h"
#include "Cube.h"
#include "Plane.h"

class MyScene : public Scene {
public:
	MyScene(GLFWwindow* window, InputHandler* H);
	~MyScene();
	void update(float dt) override;
private:
	Shader* m_myShader;
	DirectionalLight* m_directionalLight;
	std::vector<PointLight*> m_pointLight;
	SpotLight* m_spotLight;
	Cube* m_cube;
	Plane* m_plane;

	unsigned int cubeDiff;
	unsigned int cubeSpec;
	unsigned int cubeNorm;
	unsigned int planeDiff;
	unsigned int planeSpec;
	unsigned int planeNorm;

	int numCubes;
	int numPL;

	bool useNM = false;
	bool useDL = false;
	bool usePL = false;
	bool useSL = false;
		
	void render();
	void genRandCubes();
	void genRandLights();
	void toggleOptions();
};