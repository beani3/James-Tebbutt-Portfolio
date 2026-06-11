#include "windows/window.hpp";
#include "core/input.hpp"

bool InputSystem::isKeyPressed(int key) const {
	if (!m_keyStates.contains(key)) {
		return false; //!< Key not found, treat as not pressed
	}
	return m_keyStates.at(key) == GLFW_PRESS;
}

bool InputSystem::isKeyReleased(int key) const {
	if (!m_keyStates.contains(key)) {
		return false; //!< Key not found, treat as not released
	}
	return m_keyStates.at(key) == GLFW_RELEASE;
}

bool InputSystem::isKeyHeld(int key) const {
	if (!m_keyStates.contains(key)) {
		return false; //!< Key not found, treat as not held
	}
	return m_keyStates.at(key) == GLFW_REPEAT;
}

bool InputSystem::isKeyPressedPoll(int key) const
{
	return glfwGetKey(m_rawWinPtr->getGLFWwindow(), key) == GLFW_PRESS;
}

//!< Implement move function here rather than in prototype