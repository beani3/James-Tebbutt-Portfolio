/** @file renderer.cpp */
#pragma once
#include "rendering/renderer.hpp"
#include <assert.h>
#include "tracy/Tracy.hpp"
#include "tracy/TracyOpenGL.hpp"
#include <components/renderComponent.hpp>
#include <components/transformComponent.hpp>

/**
 * @brief Add a rasterization render pass to the renderer.
 * @param pass RenderPass instance to add.
 * @return Index in the renderer's render order where the pass was inserted.
 */
size_t Renderer::addRenderPass(const RenderPass& pass)
{
	size_t result = m_renderOrder.size();
	m_renderOrder.push_back(std::pair<PassType, size_t>(PassType::render, m_renderPasses.size()));
	m_renderPasses.push_back(pass);
	return result;
}

/**
 * @brief Add a depth-only render pass to the renderer.
 * @param depthPass DepthPass instance to add.
 * @return Index in the renderer's render order where the pass was inserted.
 */
size_t  Renderer::addDepthPass(const DepthPass& depthPass)
{
	size_t result = m_renderOrder.size();
	m_renderOrder.push_back(std::pair<PassType, size_t>(PassType::depth, m_depthPasses.size()));
	m_depthPasses.push_back(depthPass);
	return result;
}

/**
 * @brief Add a compute pass to the renderer.
 * @param pass ComputePass instance to add.
 * @return Index in the renderer's render order where the pass was inserted.
 */
size_t  Renderer::addComputePass(const ComputePass& pass)
{
	size_t result = m_renderOrder.size();
	m_renderOrder.push_back(std::pair<PassType, size_t>(PassType::compute, m_computePasses.size()));
	m_computePasses.push_back(pass);
	return result;
}

/**
 * @brief Get a reference to a rasterization render pass by render-order index.
 * @param index Index in the render order.
 * @return Reference to the stored RenderPass.
 * @pre isRenderPass(index) must be true.
 */
RenderPass& Renderer::getRenderPass(size_t index)
{
	assert(isRenderPass(index));
	auto& [passType, passIdx] = m_renderOrder[index];
	return m_renderPasses[passIdx];
}

/**
 * @brief Get a reference to a depth pass by render-order index.
 * @param index Index in the render order.
 * @return Reference to the stored DepthPass.
 * @pre isDepthPass(index) must be true.
 */
DepthPass& Renderer::getDepthPass(size_t index)
{
	assert(isDepthPass(index));
	auto& [passType, passIdx] = m_renderOrder[index];
	return m_depthPasses[passIdx];
}

/**
 * @brief Get a reference to a compute pass by render-order index.
 * @param index Index in the render order.
 * @return Reference to the stored ComputePass.
 * @pre isComputePass(index) must be true.
 */
ComputePass& Renderer::getComputePass(size_t index)
{
	assert(isComputePass(index));
	auto& [passType, passIdx] = m_renderOrder[index];
	return m_computePasses[passIdx];
}

/**
 * @brief Query whether the render-order entry at index is a rasterization pass.
 */
bool Renderer::isRenderPass(size_t index)
{
	return m_renderOrder.at(index).first == PassType::render;
}

/**
 * @brief Query whether the render-order entry at index is a depth pass.
 */
bool Renderer::isDepthPass(size_t index)
{
	return m_renderOrder.at(index).first == PassType::depth;
}

/**
 * @brief Query whether the render-order entry at index is a compute pass.
 */
bool Renderer::isComputePass(size_t index)
{
	return m_renderOrder.at(index).first == PassType::compute;
}

/**
 * @brief Execute all passes in the stored render order.
 *
 * @details For each pass, pre-pass actions are executed, the target FBO bound and
 * viewports / clearing set. For rasterization passes, the function iterates
 * entities with RenderComponent and TransformComponent, applies materials and
 * draws geometry. Depth passes only draw depth materials/geometry. Compute
 * passes bind image units and dispatch compute shaders; memory barriers are
 * applied if requested.
 */
void Renderer::render() const
{
	for (auto& [passType, idx] : m_renderOrder)
	{
		if (passType == PassType::render)
		{
			ZoneScopedN("RenderPass");
			TracyGpuZone("RenderPass");
			auto& renderPass = m_renderPasses[idx];

			for (auto& func : renderPass.prePassActions) func();

			{
				ZoneScopedN("FBOuse");
				renderPass.target->use();
			}
			
			{
				ZoneScopedN("SetViewPort");
				if (RenderPass::s_currViewPort != renderPass.viewPort) {
					RenderPass::s_currViewPort = renderPass.viewPort;
					glViewport(renderPass.viewPort.x, renderPass.viewPort.y, renderPass.viewPort.width, renderPass.viewPort.height);
				}
			}
			
			if (renderPass.clearDepth && renderPass.clearColour) {
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
			}
			else if (renderPass.clearDepth) {
				glClear(GL_DEPTH_BUFFER_BIT);
			}
			else if (renderPass.clearColour) {
				glClear(GL_COLOR_BUFFER_BIT);
			}

			renderPass.uploadCachedValues();

			/* Get all entities with render and transform components */
			auto view = renderPass.scene->m_entities.view<RenderComponent, TransformComponent>();
			auto camTransform = glm::inverse(renderPass.camera.view);
			glm::vec3 camPos = { camTransform[3][0], camTransform[3][1], camTransform[3][2] };

			/* Iterate over all entities in scene with render and transform components */
			view.each([camPos](const auto& renderComp, const auto& transformComp) {
				ZoneScopedN("Entity");
				TracyGpuZone("Entity");
				if (renderComp.material) {
					ZoneScopedN("Material");
					TracyGpuZone("Material");
					renderComp.material->apply();
					if (renderComp.material->getTransformUniformName().length() > 0) {
						renderComp.material->m_shader->uploadUniform(renderComp.material->getTransformUniformName(), transformComp.transform);
					}

					if (renderComp.geometry) { //!< All geometry is indexed i.e. it uses IBOs
						ZoneScopedN("Draw");
						TracyGpuZone("Draw");
						if (renderComp.geometry->getID() != s_currGeoID) {
							s_currGeoID = renderComp.geometry->getID();
							glBindVertexArray(renderComp.geometry->getID());
						}
						float dist = glm::length(transformComp.translation - camPos);

						size_t LODidx;
						if (dist > 60.f) {
							LODidx = 2;
						}
						else if (dist > 30.f) {
							LODidx = 1;
						}
						else {
							LODidx = 0;
						}

						void* baseVertexIndex = (void*)(sizeof(GLuint) * renderComp.LOD_data[LODidx].first);
						auto& drawCount = renderComp.LOD_data[LODidx].second;
						if (renderComp.toCull == false) {
							//glDrawElements(renderComp.material->getPrimitive(), renderComp.geometry->getDrawCount(), GL_UNSIGNED_INT, NULL);
							glDrawElements(renderComp.material->getPrimitive(), drawCount, GL_UNSIGNED_INT, baseVertexIndex);
						}
					}
				}
			});
		}
		else if (passType == PassType::depth)
		{
			ZoneScopedN("DepthPass");
			TracyGpuZone("DepthPass");
			{
				ZoneScopedN("FaceCullingFront");
				TracyGpuZone("FaceCullingFront");
				glCullFace(GL_FRONT);
			}
			auto& depthPass = m_depthPasses[idx];

			for (auto& func : depthPass.prePassActions) func();

			depthPass.target->use();
			glViewport(depthPass.viewPort.x, depthPass.viewPort.y, depthPass.viewPort.width, depthPass.viewPort.height);

			if (depthPass.clearDepth) {
				glClear(GL_DEPTH_BUFFER_BIT);
			}

			depthPass.uploadCachedValues();

			/* Get all entities with render and transform components */
			auto view = depthPass.scene->m_entities.view<RenderComponent, TransformComponent>();

			/** Iterate over all actors in the scene */
			for (auto entity : view)
			{
				const auto& renderComp = depthPass.scene->m_entities.get<RenderComponent>(entity);
				const auto& transformComp = depthPass.scene->m_entities.get<TransformComponent>(entity);

				if (renderComp.depthMaterial) {
					renderComp.depthMaterial->apply();
					if (renderComp.depthMaterial->getTransformUniformName().length() > 0) renderComp.depthMaterial->m_shader->uploadUniform(renderComp.depthMaterial->getTransformUniformName(), transformComp.transform);

					if (renderComp.depthGeometry) { // All geometry is indexed i.e. it uses IBOs
						glBindVertexArray(renderComp.depthGeometry->getID());
						glDrawElements(renderComp.depthMaterial->getPrimitive(), renderComp.depthGeometry->getDrawCount(), GL_UNSIGNED_INT, NULL);
					}
					//else if (renderComp.SSBOgeometry) {
					//	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, renderComp.SSBOgeometry->getID());  // binding point from material
					//	glDrawArrays(renderComp.depthMaterial->getPrimitive(), 0, renderComp.SSBOgeometry->getElementCount());
					//}
				}
			}

			for (auto& func : depthPass.postPassActions) func();

			{
				ZoneScopedN("FaceCullingBack");
				TracyGpuZone("FaceCullingBack");
				glCullFace(GL_BACK);
			}
		}
		else if (passType == PassType::compute)
		{
			auto& computePass = m_computePasses[idx];

			/** Bind images */
			for (auto& img : computePass.images) {
				GLboolean layered = img.getLayered() ? GL_TRUE : GL_FALSE;
				
				GLenum access = 0;
				switch (img.getAccess())
				{
				case TextureAccess::ReadOnly:
					access = GL_READ_ONLY; 
					break;
				case TextureAccess::WriteOnly:
					access = GL_WRITE_ONLY;
					break;
				case TextureAccess::ReadWrite:
					access = GL_READ_WRITE;
					break;
				}

				GLenum fmt = 0; //!< Ignoring depth for now
				if (img.isHDR()) {
					if (img.getChannels() == 3) fmt = GL_RGB16F;
					else fmt = GL_RGBA16F;
				}
				else {
					if (img.getChannels() == 3) fmt = GL_RGB8;
					else fmt = GL_RGBA8;
				}
				glBindImageTexture(img.getImageUnit(), img.getID(), img.getMipLevel(), layered, 0, access, fmt);
			}

			computePass.material->apply();
			auto& wg = computePass.workgroups;
			glDispatchCompute(wg.x, wg.y, wg.z);
			if(computePass.barrier != MemoryBarrier::None) glMemoryBarrier(static_cast<GLbitfield>(computePass.barrier));
		}
	}
}