#pragma once
#include <DemonRenderer.hpp>
#include <grid.hpp>

class MainLayer : public Layer
{
public:
	MainLayer(GLFWWindowImpl& win);
protected:
	void onRender() const override;
	void onUpdate(float timestep) override;
	void onImGUIRender() override;
private:
	// Scene where actors reside
	std::shared_ptr<Scene> m_scene;			
	// Scene for post-processing
	std::shared_ptr<Scene> m_screenScene;	
	// Renderer to draw the scene
	Renderer m_renderer;
	Renderer m_initPartRenderer;
	// Indexs for render passes and scene components
	size_t m_cameraIdx;						
	size_t m_shadowPPIdx;
	size_t m_computeIdx;
	size_t m_cdmIdx;
	size_t m_initPartIdx;
	size_t m_gpassIdx;						
	size_t m_blurIdx;
	size_t m_depthIdx;
	size_t m_fogIdx;
	size_t m_dofIdx;
	size_t m_shadowIdx;
	size_t m_lightIdx;
	size_t m_skyboxIdx;
	int m_floorIdx{ -1 };
	int m_modelIdx{ -1 };
	int m_sbIdx{ -1 };
	int m_quadIdx{ -1 };
	// Model & model textures
	Actor m_model;
	std::shared_ptr<Texture> m_modelAlbedo;
	std::shared_ptr<Texture> m_modelHeight;
	std::shared_ptr<Texture> m_modelMetallic;
	std::shared_ptr<Texture> m_modelNormal;
	std::shared_ptr<Texture> m_modelRoughness;
	std::shared_ptr<Texture> m_floorAlbedo;
	std::shared_ptr<Texture> m_floorNormal; 
	std::shared_ptr<Texture> m_floorMetallic;
	std::shared_ptr<Texture> m_floorRoughness;
	float m_deltaTime;
	float m_previousTime;

	// Scene variables
	glm::vec3 m_dlDirection = glm::vec3(1.f, -2.5f, -2.f);
	glm::vec2 m_imageSize;
	// Passes
	DepthPass shadowPass;
	// Materials
	std::shared_ptr<Material> m_lightPassMaterial;
	std::shared_ptr<Material> m_postProcessingMaterial;
	std::shared_ptr<Material> m_floorMaterial;
	// Shadow map variables
	struct ShadowMapVars {
		glm::vec3 centre;
		float distanceAlongLightVector;
		float orthoSize;
		const glm::vec3 UP = glm::vec3(0.f, 1.f, 0.f);

		ShadowMapVars(glm::vec3 c = glm::vec3(0.f), float dist = 60.f) : centre(c), distanceAlongLightVector(dist) {
			orthoSize = distanceAlongLightVector * .75f;
		}
	};

	glm::mat4 shadowView;
	ShadowMapVars m_shadowMapVars;
	glm::vec3 m_floorColour = glm::vec3(0.25f);
	static const uint32_t numPointLights = 100;
	// Particle variables
	struct Particle {
		glm::vec4 velocity;
		glm::vec4 position;
		float age;
	};

	float m_numParts = 10000;
	// Gui
	bool m_wireFrame{ false }; // render in wireframe
	glm::vec3 m_attn = glm::vec3(0.125f, 0.11f, 0.06f);
	float m_near = 0.1f;
	float m_far = 10.f;
	float m_sf = 7.5f;
	int m_radius = 5;
	int m_octaves = 2;
	float m_frequency = 2.5f;
	float m_amplitude = 1.5f;
	float m_persistance = 0.5;
	float m_lacunarity = 2.0f;
	float m_ambientFactor = 0.01f;
	float m_metallic = 0.3f;;
	float m_roughness = 0.3f;
	// Toggle bools
	bool m_textured{ false };
	int m_normalToggle = -1;
	// Skybox
	Actor m_skybox;
	std::vector<float> skyboxVertices = {
		// positions          
		-100.f,  100.f, -100.f,
		-100.f, -100.f, -100.f,
		 100.f, -100.f, -100.f,
		 100.f, -100.f, -100.f,
		 100.f,  100.f, -100.f,
		-100.f,  100.f, -100.f,

		-100.f, -100.f,  100.f,
		-100.f, -100.f, -100.f,
		-100.f,  100.f, -100.f,
		-100.f,  100.f, -100.f,
		-100.f,  100.f,  100.f,
		-100.f, -100.f,  100.f,

		 100.f, -100.f, -100.f,
		 100.f, -100.f,  100.f,
		 100.f,  100.f,  100.f,
		 100.f,  100.f,  100.f,
		 100.f,  100.f, -100.f,
		 100.f, -100.f, -100.f,

		-100.f, -100.f,  100.f,
		-100.f,  100.f,  100.f,
		 100.f,  100.f,  100.f,
		 100.f,  100.f,  100.f,
		 100.f, -100.f,  100.f,
		-100.f, -100.f,  100.f,

		-100.f,  100.f, -100.f,
		 100.f,  100.f, -100.f,
		 100.f,  100.f,  100.f,
		 100.f,  100.f,  100.f,
		-100.f,  100.f,  100.f,
		-100.f,  100.f, -100.f,

		-100.f, -100.f, -100.f,
		-100.f, -100.f,  100.f,
		 100.f, -100.f, -100.f,
		 100.f, -100.f, -100.f,
		-100.f, -100.f,  100.f,
		 100.f, -100.f,  100.f
	};
};