#include <pch.h>

#include "Parameter.h"
#include "Parameters/StringParameter.h"

namespace Legato{

void Parameter::onConstruction() {
	Component::onConstruction();
	//All parameters own a string parameter to hold their component name
	//this way each parameter can be renamed easily
	//this string parameter owned by parameters does not in turn have a parametric name
	//instead it just has a a string as a parameter that reads "Parameter Name"
	//When a parameter gets its name set, we also update its imguiID to comply with the gui library
	if(b_hasNameParameter){
		nameParameter->addEditCallback([this](){
			std::string parameterName = getName();
			imGuiID = "##" + parameterName;
		});
	}
}

void ParameterGroup::gui(){
	
	if(ImGui::CollapsingHeader(getName().c_str())){
		
		if(ImGui::BeginTable("##parameters", 2, ImGuiTableFlags_RowBg)){
			
			ImGui::TableSetupColumn("Parameter");
			ImGui::TableSetupColumn("Value");
			
			float frameHeight = ImGui::GetFrameHeight();
			ImVec2 offset(ImGui::GetStyle().CellPadding.y, (frameHeight - ImGui::GetTextLineHeight()) / 2.0);
			
			for(auto parameter : getParameters()){
				ImGui::TableNextRow();
				ImGui::TableSetColumnIndex(0);
				ImVec2 cursorPos = ImGui::GetCursorPos();
				ImGui::SetCursorPos(ImVec2(cursorPos.x + offset.x, cursorPos.y + offset.y));
				
				ImGui::BeginDisabled(parameter->isDisabled());
				ImGui::Text("%s", parameter->getName().c_str());
				ImGui::EndDisabled();
				
				ImGui::TableSetColumnIndex(1);
				ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x - ImGui::GetStyle().CellPadding.y);
				parameter->gui();
			}
			
			ImGui::EndTable();
		}
	}
}

}
