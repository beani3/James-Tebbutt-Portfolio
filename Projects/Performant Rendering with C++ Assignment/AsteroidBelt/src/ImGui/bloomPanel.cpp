#include "ImGui/bloomPanel.hpp"

BloomPanel::BloomPanel(Renderer* renderer, std::shared_ptr<Texture> downSampleTex, std::shared_ptr<Texture> upSampleTex) :
	m_renderer(renderer),
	m_downSampleTex(downSampleTex),
	m_upSampleTex(upSampleTex)
{
	ShaderDescription debugDesc;
	debugDesc.type = ShaderType::rasterization;
	debugDesc.vertexSrcPath = "./assets/shaders/Bloom/debugVert.glsl";
	debugDesc.fragmentSrcPath = "./assets/shaders/Bloom/debugFrag.glsl";

	m_debugShader = std::make_shared<Shader>(debugDesc);

	const std::vector<uint32_t> debugIndices = { 0,1,2,2,3,0 };
	VBOLayout debugQuadLayout = {
		{GL_FLOAT, 2},
		{GL_FLOAT, 2}
	};
	std::vector<float> debugVertices = {
		// Position		UV
		-100.f, -100.f,		0.f, 1.f,
		611.f, -100.f,		1.f, 1.f,
		611.f, 300.f,		1.f, 0.f,
		-100.f, 300.f,		0.f, 0.f
	};

	m_debugVAO = std::make_shared<VAO>(debugIndices);
	m_debugVAO->addVertexBuffer(debugVertices, debugQuadLayout);

	m_thresholdPassInfo = generatePassInfo({ 711,400 }, 0);

	m_downPasses[1] = generatePassInfo({ 711,400 }, 1);
	m_downPasses[2] = generatePassInfo({ 356,200 }, 2);
	m_downPasses[3] = generatePassInfo({ 178,100 }, 3);
	m_downPasses[4] = generatePassInfo({ 89,50 }, 4);
	m_downPasses[5] = generatePassInfo({ 45,25 }, 5);

	m_upPasses[4] = generatePassInfo({ 89,50 }, 4);
	m_upPasses[3] = generatePassInfo({ 178,100 }, 3);
	m_upPasses[2] = generatePassInfo({ 356,200 }, 2);
	m_upPasses[1] = generatePassInfo({ 711,400 }, 1);
	m_upPasses[0] = generatePassInfo({ 711,400 }, 0);
}
void BloomPanel::onImGuiRender()
{
	if (ImGui::TreeNode("Bloom controls"))
	{
		if (m_renderer) {

		/*	if (ImGui::DragFloat("Bloom Strength", &m_strength, 0.005f, 0.f, 1.f))
			{
				auto& compositionMaterial = m_renderer->getRenderPass(12).scene->m_entities[0].material;
				compositionMaterial->setValue("u_bloomStrength", m_strength);
			}

			if (ImGui::DragFloat("Bloom Th", &m_bloomThreshold, 0.05f, 50.f, 0.f))
			{
				auto& thresholdMaterial = m_renderer->getRenderPass(1).scene->m_entities[0].material;
				thresholdMaterial->setValue("u_threshold", m_bloomThreshold);
			}*/

			if (ImGui::TreeNode("Threshold Texture"))
			{
				auto texId = renderPassMipmapLevel(m_thresholdPassInfo, m_renderer->getRenderPass(1).target->getTarget(0), 0);
				ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
				ImGui::TreePop();
			}
		}
		else {
			ImGui::Text("Renderer not set.");
		}
		
		if (ImGui::TreeNode("Downsample Textures"))
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("Downsample Textures", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Downsample 1"))
				{
					auto texId = renderDownpassMipmapLevel(1);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Downsample 2"))
				{
					auto texId = renderDownpassMipmapLevel(2);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Downsample 3"))
				{
					auto texId = renderDownpassMipmapLevel(3);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Downsample 4"))
				{
					auto texId = renderDownpassMipmapLevel(4);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Downsample 5"))
				{
					auto texId = renderDownpassMipmapLevel(5);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::TreePop();
		}

		if (ImGui::TreeNode("Upsample Textures"))
		{
			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None;
			if (ImGui::BeginTabBar("Upsample Textures Tabbar", tab_bar_flags))
			{
				if (ImGui::BeginTabItem("Upsample 1"))
				{
					auto texId = renderUppassMipmapLevel(4);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Upsample 2"))
				{
					auto texId = renderUppassMipmapLevel(3);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Upsample 3"))
				{
					auto texId = renderUppassMipmapLevel(2);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Upsample 4"))
				{
					auto texId = renderUppassMipmapLevel(1);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Upsample 5"))
				{
					auto texId = renderUppassMipmapLevel(0);
					ImGui::Image(ImTextureID(texId), ImVec2(711, 400));
					ImGui::EndTabItem();
				}

				ImGui::EndTabBar();
			}
			ImGui::TreePop();
		}

		ImGui::Separator();
		ImGui::TreePop();
	}
}

uint32_t BloomPanel::renderDownpassMipmapLevel(GLint mipLevel) {
	auto it = m_downPasses.find(mipLevel);
	if (it == m_downPasses.end()) return 0; // Mip level not found

	return renderPassMipmapLevel(it->second, m_downSampleTex, mipLevel);
}

uint32_t BloomPanel::renderUppassMipmapLevel(GLint mipLevel)
{
	auto it = m_upPasses.find(mipLevel);
	if (it == m_upPasses.end()) return 0; // Mip level not found

	return renderPassMipmapLevel(it->second, m_upSampleTex, mipLevel);
}

uint32_t BloomPanel::renderPassMipmapLevel(const PassInfo& passInfo, std::shared_ptr<Texture> srctex, GLint mipLevel)
{
	passInfo.fbo->use();

	glUseProgram(m_debugShader->getID());
	m_debugShader->uploadUniform<float>("mipLevel", (float)mipLevel);
	glBindTextureUnit(1, srctex->getID());
	m_debugShader->uploadUniform<int>("tex", 1);

	glBindVertexArray(m_debugVAO->getID());

	glViewport(0, 0, passInfo.size.x, passInfo.size.y);  // Set the viewport to the mipmap level size
	glClear(GL_COLOR_BUFFER_BIT);      // Clear framebuffer
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, NULL); // Render a quad (ensure you have the correct shader and VAO)

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	return passInfo.fbo->getTarget(0)->getID();
}

BloomPanel::PassInfo BloomPanel::generatePassInfo(const glm::ivec2& size, uint32_t mip)
{
	TextureDescription debugTexDesc;
	debugTexDesc.channels = 4;
	debugTexDesc.type = TextureDataType::HDR;
	debugTexDesc.width = size.x;
	debugTexDesc.height = size.y;
	debugTexDesc.minFilterMethod = GL_NEAREST;
	debugTexDesc.magFilterMethod = GL_NEAREST;

	std::shared_ptr<Texture> debugTex = std::make_shared<Texture>(debugTexDesc);

	FBOLayout simpleLayout = {
		{debugTex, AttachmentType::ColourHDR, true, false, 0}
	};

	PassInfo passInfo;
	passInfo.fbo = std::make_shared<FBO>(size, simpleLayout);
	passInfo.size = size;
	return passInfo;
}
