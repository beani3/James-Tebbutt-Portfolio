/** @file UBObasePass.cpp */
#include "rendering/UBObasePass.hpp"
#include "core/application.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

/**
 * @brief Upload cached uniform values (grouped by UBO block) to GPU.
 *
 * @details Iterates the per-pass cache, resolves which UBO binding point each cached
 * uniform belongs to and invokes the application's UBO manager to perform the
 * per-uniform upload using the correct offset/size information.
 *
 * Supported cached types are currently:
 *  - GL_FLOAT
 *  - GL_FLOAT_VEC3
 *  - GL_FLOAT_MAT4
 *
 * @note This function expects the cache layout to contain (blockName -> (uniformName, UniformInfo))
 */
void UBOCacheBasePass::uploadCachedValues() const
{
	ZoneScopedN("UBO");
	TracyGpuZone("UBO");

	/** Group cached uniforms per UBO block */
	for (auto& dataPair : m_cache)
	{
		auto& blockName = dataPair.first;
		auto it = m_UBOmap.find(blockName);
		if (it == m_UBOmap.end()) {
			spdlog::error("Uploading UBO cache. Can't find block {}", blockName);
			continue;
		}
		
		auto& bindingPoint = it->second;

		/** Cached uniform info */
		auto& uniformName = dataPair.second.first;
		auto& info = dataPair.second.second;

		/** Copy data into buffer */
		switch (info.type)
		{
		case GL_FLOAT:
		{
			float _float = std::get<float>(info.data);
			Application::UBOmanager.uploadUniform(bindingPoint, uniformName, &_float);
		}
		break;

		case GL_FLOAT_VEC3:
			glm::vec3 vec3 = std::get<glm::vec3>(info.data);
			Application::UBOmanager.uploadUniform(bindingPoint, uniformName, glm::value_ptr(vec3));
			break;

		case GL_FLOAT_MAT4:
			glm::mat4 mat4 = std::get<glm::mat4>(info.data);
			Application::UBOmanager.uploadUniform(bindingPoint, uniformName, glm::value_ptr(mat4));
			break;
		}
	}
}

/**
 * @brief Retrieve the UBO layout for a given binding point.
 * @param bindingPoint Binding point to query.
 * @param layout Out parameter that will be filled with the layout if found.
 * @return true if the layout was found, false otherwise.
 */
bool UBOCacheBasePass::getLayout(size_t bindingPoint, UBOLayout& layout) const
{
	bool gotLayout = Application::UBOmanager.getLayout(bindingPoint, layout);
	return gotLayout;
}
