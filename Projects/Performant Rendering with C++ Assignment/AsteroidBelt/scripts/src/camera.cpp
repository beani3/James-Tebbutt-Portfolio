#include "camera.hpp"
#include <components/transformComponent.hpp>

void CameraScript::onUpdate(float timestep)
{
    auto& transformComp = m_registry.get<TransformComponent>(m_entity);

    bool recalc = false;

    glm::vec3 right = glm::normalize(glm::vec3(transformComp.transform[0][0], transformComp.transform[0][1], transformComp.transform[0][2]));
    glm::vec3 forward = glm::normalize(glm::vec3(-transformComp.transform[2][0], -transformComp.transform[2][1], -transformComp.transform[2][2]));

    if (m_winRef.doIsKeyPressed(GLFW_KEY_W)) { transformComp.translation += forward * m_movementSpeed.z * timestep; recalc = true; }
    if (m_winRef.doIsKeyPressed(GLFW_KEY_A)) { transformComp.translation -= right * m_movementSpeed.x * timestep; recalc = true; }
    if (m_winRef.doIsKeyPressed(GLFW_KEY_S)) { transformComp.translation -= forward * m_movementSpeed.z * timestep; recalc = true; }
    if (m_winRef.doIsKeyPressed(GLFW_KEY_D)) { transformComp.translation += right * m_movementSpeed.x * timestep; recalc = true; }
    if (m_winRef.doIsKeyPressed(GLFW_KEY_UP)) { transformComp.translation.y += m_movementSpeed.y * timestep; recalc = true; }
    if (m_winRef.doIsKeyPressed(GLFW_KEY_DOWN)) { transformComp.translation.y -= m_movementSpeed.y * timestep; recalc = true; }

    if (m_winRef.doIsKeyPressed(GLFW_KEY_LEFT)) {
        transformComp.rotation = glm::rotate(transformComp.rotation, m_turnSpeed * timestep, glm::vec3(0, 1, 0));
        recalc = true;
    }
    if (m_winRef.doIsKeyPressed(GLFW_KEY_RIGHT)) {
        transformComp.rotation = glm::rotate(transformComp.rotation, -m_turnSpeed * timestep, glm::vec3(0, 1, 0));
        recalc = true;
    }

    if (recalc) {
        transformComp.recalc();
    }
}