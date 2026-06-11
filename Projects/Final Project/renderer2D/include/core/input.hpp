#pragma once
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>
#include <unordered_map>

class Window;

class InputSystem {
public:
	inline static InputSystem& get() {
		static InputSystem s_instance;
		return s_instance;
	}
	bool isKeyPressed(int key) const;
	bool isKeyReleased(int key) const;
	bool isKeyHeld(int key) const;
	bool isKeyPressedPoll(int key) const;
	std::unordered_map<int, int> m_keyStates;
	Window* m_rawWinPtr;
};