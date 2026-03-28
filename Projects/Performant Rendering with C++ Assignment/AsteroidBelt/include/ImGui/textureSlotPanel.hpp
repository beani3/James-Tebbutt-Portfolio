/** \file textureSlotPanel */
#pragma once
#include "DemonRenderer.hpp"

class TextureSlotPanel
{
public:
	TextureSlotPanel();
	void onImGuiRender();
private:
	std::shared_ptr<ManagedTexture> m_emptyTexture;
};
