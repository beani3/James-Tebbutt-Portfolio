/** @file material.cpp */
#include "rendering/material.hpp"
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"

/**
 * @brief Construct a Material by scanning the shader's uniform info cache.
 * @param shader Shared pointer to the Shader instance.
 * @param transformUniformName Optional name of the model/transform uniform to be uploaded per-object.
 *
 * @details The constructor builds the material's data cache based on the shader's uniform
 * metadata so subsequent material value assignments and uploads can be efficiently
 * dispatched.
 */
Material::Material(std::shared_ptr<Shader> shader, const std::string& transformUniformName) : 
	m_shader(shader),
	m_transformUniformName(transformUniformName)
{
	auto& infoCache = shader->m_uniformInfoCache;

	for (auto it = infoCache.begin(); it != infoCache.end(); ++it)
	{
		auto& name = it->first;
		auto& info = it->second;

		dataCache[name] = UniformData(info.type, info.size);

	}
}

/**
 * @brief Disable a previously set material uniform value.
 * @param name Name of the uniform to unset.
 *
 * If the uniform name does not exist in the cache an error will be logged.
 */
void Material::unsetValue(const std::string& name)
{
	auto it = dataCache.find(name);
	if (it != dataCache.end()) {
		auto& data = dataCache[name];
		data.enabled = false;
	}
	else { 
		spdlog::error("Could not set material value: {}. Not found in shader data cache.", name); }	
}

/**
 * @brief Apply the material: bind shader and upload enabled uniform values.
 *
 * @details Iterates the material's data cache and uses the dispatch table in UniformConsts
 * to upload each enabled uniform in the correct manner based on its GL type.
 */
void Material::apply()
{
	ZoneScopedN("Material");
	TracyGpuZone("Material");
	
	/** Bind shader */
	{
		ZoneScopedN("BindShader");
		if (s_currShaderID != m_shader->getID())
		{
			s_currShaderID = m_shader->getID();
			glUseProgram(m_shader->getID());
		}
	}

	/** Upload uniforms */
	for (auto& dataPair : dataCache)
	{
		ZoneScopedN("BindUniform");
		auto& name = dataPair.first;
		auto& matInfo = dataPair.second;

		if (matInfo.enabled)
		{
			auto& dispatchFunc = UniformConsts::UDT.at(matInfo.type);

			dispatchFunc(m_shader, name, matInfo);
			
			/*
			switch (matInfo.type)
			{
			case GL_FLOAT_VEC3:
				m_shader->uploadUniform(name, std::get<glm::vec3>(matInfo.data));
				break;
			case GL_SAMPLER_2D:
				m_shader->uploadUniform(name, std::get<std::shared_ptr<Texture>>(matInfo.data)->getUnit());
				break;
			}*/
		}
	}
}