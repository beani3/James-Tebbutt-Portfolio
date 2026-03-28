#pragma once
#include "DemonRenderer.hpp"

class BloomPanel
{
public:
	BloomPanel() {};
	BloomPanel(Renderer* renderer, std::shared_ptr<Texture> downSampleTex, std::shared_ptr<Texture> upSampleTex);
	void onImGuiRender();
	[[nodiscard]] float getThreshold() const { return m_bloomThreshold; }
	[[nodiscard]] float getStrength() const { return m_strength; }
private:
	struct PassInfo {
		std::shared_ptr<FBO> fbo;
		glm::ivec2 size;
	};
private:
	[[nodiscard]] uint32_t renderDownpassMipmapLevel(GLint mipLevel);
	[[nodiscard]] uint32_t renderUppassMipmapLevel(GLint mipLevel);
	[[nodiscard]] uint32_t renderPassMipmapLevel(const PassInfo& passInfo, std::shared_ptr<Texture> srctex, GLint mipLevel);
	[[nodiscard]] PassInfo generatePassInfo(const glm::ivec2& size, uint32_t mip);
private:
	Renderer* m_renderer{ nullptr };
	std::shared_ptr<Texture> m_downSampleTex;
	std::shared_ptr<Texture> m_upSampleTex;
	std::shared_ptr<Shader> m_debugShader;
	std::shared_ptr<VAO> m_debugVAO;
	std::unordered_map<GLint,PassInfo> m_downPasses; //!< Maps mips levels to info needed to debug render that downpass.
	std::unordered_map<GLint,PassInfo> m_upPasses; //!< Maps mips levels to info needed to debug render that uppass.
	PassInfo m_thresholdPassInfo;
	float m_bloomThreshold{ 1.5f };
	float m_strength{ 0.15f };
};