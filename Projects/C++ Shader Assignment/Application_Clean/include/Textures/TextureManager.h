#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

class TextureManager {
public:
	TextureManager();
	~TextureManager();
	static unsigned int LoadTexture(char const* path);
};