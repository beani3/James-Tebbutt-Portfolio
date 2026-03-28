#include "followCamera.hpp"
#include <components/transformComponent.hpp>

void FollowCameraScript::onUpdate(float timestep)
{
	auto& transformComp = m_registry.get<TransformComponent>(m_entity);

	glm::vec3 targetRight = { transformComp.transform[0][0], transformComp.transform[0][1], transformComp.transform[0][2] };
	glm::vec3 targetUp = { transformComp.transform[1][0], transformComp.transform[1][1], transformComp.transform[1][2] };
	glm::vec3 targetForward = { transformComp.transform[2][0], transformComp.transform[2][1], transformComp.transform[2][2] };
	// glm::vec3 targetTranslation = { transformComp.transform[3][0], transformComp.transform[3][1], transformComp.transform[3][2] };

	transformComp.translation += targetRight * m_offset.x;
	transformComp.translation += targetUp * m_offset.y;
	transformComp.translation += targetForward * m_offset.z;

	auto& followTransformComp = m_registry.get<TransformComponent>(m_target);
	followTransformComp.translation = transformComp.translation;

	followTransformComp.translation += targetRight * m_offset.x;
	followTransformComp.translation += targetUp * m_offset.y;
	followTransformComp.translation += targetForward * m_offset.z;

	followTransformComp.rotation = transformComp.rotation * glm::quat(glm::vec3(0.f, glm::pi<float>(), 0.f)); // Hack as model is backward to start with

	followTransformComp.recalc();
}

