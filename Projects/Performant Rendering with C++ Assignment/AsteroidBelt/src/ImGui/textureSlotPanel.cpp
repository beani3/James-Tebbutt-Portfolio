/** \file textureSlotPanel.cpp */
#include "ImGui/textureSlotPanel.hpp"

TextureSlotPanel::TextureSlotPanel()
{
	unsigned char pxData[4] = { 0,0,0,0 };

	TextureDescription td;
	td.channels = 4;
	td.height = 1;
	td.width = 1;
	
	m_emptyTexture = std::make_shared<Texture>(td, pxData);
}

void TextureSlotPanel::onImGuiRender()
{
	if (ImGui::TreeNode("Texture Slot Occupancy"))
	{
		ImGui::BeginTable("Texture Slots", 9, ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg);
		for (size_t row = 0; row < 4; row++)
		{
			ImGui::TableNextRow();
			for (int column = 0; column < 9; column++)
			{
				ImGui::TableSetColumnIndex(column);
				if (column == 0) ImGui::Text("%d", column + 8 * row);
				else
				{
					int value = m_emptyTexture->inspectSlot(column + 8 * row - 1);
					if(value >= 0) ImGui::Text("%d", value);
					else ImGui::Text("-");
				}
			}
		}
		ImGui::EndTable();

		ImGui::TreePop();
	}
}
