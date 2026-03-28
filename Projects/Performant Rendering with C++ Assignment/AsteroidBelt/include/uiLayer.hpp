/** @file uiLayer.hpp */
#pragma once
/** UI rendering layer, use DemonRenderer but also use programmable vertex pulling */
#include <DemonRenderer.hpp>
#include"sharedData.hpp"
#include <entt/entt.hpp>

struct QuadVertex {
	glm::vec2 position;
	glm::vec2 texCoords;
	glm::vec4 colour;
	float texSlot;
};

struct CircleVertex {
	glm::vec2 position;
	glm::vec2 unitPosition;
	glm::vec4 colour;
	float thickness;
};

class UI : public Layer
{
public:
	UI(GLFWWindowImpl& win, SharedData& data);
	void onRender() const;
	void onUpdate(float timestep) override;
private:
	void begin(); //!< Clear all quads, circles and rounded quads
	void end(); //!< Render all quads, circles and rounded quads drawn since begin
	void drawQuad(const glm::vec2& position, const glm::vec2& size, const glm::vec4& colour); //!< Draw a quad at position (top-left) with size (width and height)
	void drawTexturedQuad(const glm::vec2& position, const glm::vec2& size, std::shared_ptr<Texture> texture); //!< Draw a quad at position (top-left) with size (width and height) and texture
	void drawCircle(const glm::vec2& centre, float radius, const glm::vec4& colour, float thickness); //!< Draw a circle at centre with radius;
private:
	LOD_data screenLODs;

	SharedData& m_gameData; //!< Game data shread across layers
	Renderer m_UIRenderer; //!< Renderer to draw the scene
	std::shared_ptr<Scene> m_UIScene; //!< Scene with three actors, quads, circles and rounded quads

	std::shared_ptr<SSBO> m_quadsSSBO; //!< SSBO to hold quad vertex data
	entt::entity m_quads; 

	static const int m_maxQuadCount = 256; //!< Maximum number of quads draws in the UI widget
	std::array<QuadVertex, m_maxQuadCount * 4> m_quadVertices; //!< Quad vertices, CPU side
	size_t m_currentQuadCount{ 0 }; //!< Number of quads currently set to be rendered
	size_t m_quadIdx{ 0 }; //!< Index of next quad

	std::shared_ptr<SSBO> m_circlesSSBO; //!< SSBO to hold circle vertex data
	entt::entity m_circles; 

	static const int m_maxCircleCount = 256; //!< Maximum number of circles draws in the UI widget
	std::array<CircleVertex, m_maxCircleCount * 4> m_circleVertices; //!< Circle vertices, CPU side
	size_t m_currentCircleCount{ 0 }; // Number of circles currently set to be rendered
	size_t m_circleIdx{ 0 }; //!< Index of next circle

	/** Set of texture slots */
	std::array<int, 32> m_slots = { 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31 };
	std::shared_ptr<Texture> m_defaultTexture; //!< Used for drawing an untextured quad
	const glm::vec4 m_defaultColour{ glm::vec4(1.f) }; //!< Used for drawing a textured quad

	std::shared_ptr<Texture> m_introTexture{ nullptr }; //!< Intro texture
	std::shared_ptr<Texture> m_gameOverTexture{ nullptr }; //!< Game over texture
	/** Set of colours for rendering speed widget */
	const std::array<glm::vec4, 16> m_speedUIColours = {
		glm::vec4(0.992156862745098f, 0.941176470588235f, 0.0117647058823529f, 0.85f),
		glm::vec4(0.984313725490196f, 0.894117647058824f, 0.0313725490196078f, 0.85f),
		glm::vec4(0.976470588235294f, 0.83921568627451f, 0.0549019607843137f, 0.85f),
		glm::vec4(0.972549019607843f, 0.788235294117647f, 0.0745098039215686f, 0.85f),
		glm::vec4(0.964705882352941f, 0.737254901960784f, 0.0941176470588235f, 0.85f),
		glm::vec4(0.956862745098039f, 0.686274509803922f, 0.113725490196078f, 0.85f),
		glm::vec4(0.949019607843137f, 0.631372549019608f, 0.137254901960784f, 0.85f),
		glm::vec4(0.941176470588235f, 0.580392156862745f, 0.156862745098039f, 0.85f),
		glm::vec4(0.933333333333333f, 0.529411764705882f, 0.176470588235294f, 0.85f),
		glm::vec4(0.933333333333333f, 0.462745098039216f, 0.180392156862745f, 0.85f),
		glm::vec4(0.945098039215686f, 0.337254901960784f, 0.133333333333333f, 0.85f),
		glm::vec4(0.96078431372549f, 0.203921568627451f, 0.0784313725490196f, 0.85f),
		glm::vec4(0.972549019607843f, 0.0784313725490196f, 0.0313725490196078f, 0.85f),
		glm::vec4(0.968627450980392f, 0.f,0.f, 0.85f),
		glm::vec4(0.937254901960784f, 0.f,0.f, 0.85f),
		glm::vec4(0.909803921568627f, 0.f,0.f, 0.85f)
	};

	/** Set of thresholds for rendering speed widget */
	const std::array<float, 15> m_speedThresholds = {
			-0.82f,
			-1.14f,
			-1.46f,
			-1.78f,
			-2.1f,
			-2.42f,
			-2.74f,
			-3.06f,
			-3.38f,
			-3.7f,
			-4.02f,
			-4.34f,
			-4.66f,
			-4.98f,
			-5.3f
	};
};
