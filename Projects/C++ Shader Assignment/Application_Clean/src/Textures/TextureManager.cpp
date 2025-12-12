#pragma once
#include "Textures/TextureManager.h"
#include <iostream>
#define STB_IMAGE_IMPLEMENTATION
#include "Textures/stb_image.h"

TextureManager::TextureManager() {

}

TextureManager::~TextureManager() {}

unsigned int TextureManager::LoadTexture(char const* path)
{
	unsigned int textureID;
	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);

	if (data)
	{
		GLenum format;
		if (nrComponents == 1)     format = GL_RED;
		else if (nrComponents == 3)format = GL_RGB;
		else if (nrComponents == 4)format = GL_RGBA;

		glCreateTextures(GL_TEXTURE_2D, 1, &textureID);
		glTextureStorage2D(textureID, 4, GL_RGBA8, width, height);
		glTextureSubImage2D(textureID, 0, 0, 0, width, height, format, GL_UNSIGNED_BYTE, data);
		glGenerateTextureMipmap(textureID);


		glBindTexture(GL_TEXTURE_2D, textureID);
		glTextureParameteri(textureID, GL_TEXTURE_WRAP_S, GL_REPEAT); // S = x axis 
		glTextureParameteri(textureID, GL_TEXTURE_WRAP_T, GL_REPEAT); // T == y axis, R if 3D
		glTextureParameteri(textureID, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTextureParameteri(textureID, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
		std::cout << "Loaded texture at path: " << path << " components " << nrComponents << " id " << textureID << std::endl;
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}
	return textureID;
}