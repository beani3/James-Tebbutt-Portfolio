#pragma once
#include <DemonRenderer.hpp>
#include <entt/entt.hpp>

class LOD : public Layer
{
public:
	LOD(GLFWWindowImpl& win);
	void onRender() const override;
	void onUpdate(float timestep) override;
	void onKeyPressed(KeyPressedEvent& e) override;
	void onImGUIRender() override;
private:
	std::shared_ptr<Scene> m_mainScene;
	Renderer m_mainRenderer;
	RenderPass m_mainPass;
	bool m_wireframe{ false };

	/** LOD variables */
	std::vector<uint32_t> m_indices;
	std::vector<float> m_vertices;
	std::vector<uint32_t> m_indicesLOD1;
	std::vector<uint32_t> m_indicesLOD2;
	entt::entity allLODs{ entt::null };
	std::array<std::pair<uint32_t, uint32_t>, 3> LOD_data;
	size_t LODidx{ 0 };
};