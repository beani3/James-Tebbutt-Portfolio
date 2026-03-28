#include "rendering/depthOnlyPass.hpp"
#include "core/application.hpp"
#include <components/renderComponent.hpp>

void DepthPass::parseScene()
{
	auto renderView = scene->m_entities.view<RenderComponent>(); //!< Get all entities with render component 

	for (auto& entity : renderView)
	{
		auto& renderComp = renderView.get<RenderComponent>(entity); //!< Get render component by ref

		if (renderComp.material) {
			if (renderComp.depthMaterial) {
				for (auto& [blockname, layout] : renderComp.depthMaterial->m_shader->m_UBOLayouts) {
					// Add blockname lookup
					auto UBOmapIt = m_UBOmap.find(blockname);
					if (UBOmapIt == m_UBOmap.end()) {
						// Insert lookup
						m_UBOmap.insert({blockname, layout.getBindingPoint()});
					}
					else {
						if (UBOmapIt->second != layout.getBindingPoint()) {
							spdlog::error("Error - attempting to add to UBO blocks with the same name to different binding points!");
						}
					}
			
					// Add UBO
					Application::UBOmanager.add(layout);
				}
			}
		}
	}
}
