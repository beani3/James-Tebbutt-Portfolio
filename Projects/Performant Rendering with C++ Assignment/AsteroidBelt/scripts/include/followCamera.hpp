#pragma once

#include "DemonRenderer.hpp"
#include <components/scriptComponent.hpp>

class FollowCameraScript : public Script
{
public:
	FollowCameraScript(entt::entity entity, std::shared_ptr<Scene> scene, GLFWWindowImpl& win, entt::entity& target, const glm::vec3& offset) :
		Script(entity, scene),
		m_winRef(win),
		m_target(target),
		m_offset(offset)
	{}
	virtual void onUpdate(float timestep) override;
	virtual void onKeyPress(KeyPressedEvent& e) override {};
	virtual void onKeyRelease(KeyReleasedEvent& e) override {};
	void setOffsetZ(float z) { m_offset.z = z; }
	void setOffsetY(float y) { m_offset.z = y; }
private:
	GLFWWindowImpl& m_winRef;
	entt::entity& m_target;
	glm::vec3 m_offset;
};
