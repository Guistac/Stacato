#include <pch.h>

#include "Manoeuvre.h"

#include <imgui.h>
#include "Gui/Utilities/CustomWidgets.h"

namespace AnimationSystem{

	void Manoeuvre::editorGui(){

		
		ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, glm::vec2(ImGui::GetTextLineHeight() * 0.15));
		ImGuiTableFlags tableFlags = ImGuiTableFlags_Borders | ImGuiTableFlags_SizingFixedFit | ImGuiTableFlags_ScrollX;
	

		if(ImGui::BeginTable("##animations", 2, tableFlags)){
			
			ImGui::TableSetupColumn("Animatable");
			ImGui::TableSetupColumn("Parameters");
			ImGui::TableHeadersRow();
			
			for(int i = 0; i < animations.size(); i++){
				auto animation = animations[i];
				
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImGui::Text("Name of Animatable");
				ImGui::TableSetColumnIndex(1);
				ImGui::Text("Animation Parameters");
				
			}
			
			
			ImGui::EndTable();
		}
		
		
		
		
		
		ImGui::PopStyleVar(2);
	}

};
