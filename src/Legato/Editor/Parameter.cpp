#include <pch.h>

#include "Parameter.h"
#include "Parameters/StringParameter.h"

namespace Legato{

void Parameter::onConstruction() {
	//parameters don't have a name parameter
}

bool Parameter::onSerialization() {
	//parameters don't have a name parameter
	return true;
}
bool Parameter::onDeserialization() {
	//parameters don't have a name parameter
	return true;
}

void Parameter::setName(std::string name){
	if(b_hasNameParameter) {
		nameParameter->overwrite(name);
		nameParameter->onEdit();
	}
	else nonParametricName = name;
	imGuiID = "##" + name;
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
